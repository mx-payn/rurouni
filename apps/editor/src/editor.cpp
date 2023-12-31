// editor
#include "rurouni/editor.hpp"
#include "magic_enum.hpp"
#include "msdf-atlas-gen/GlyphGeometry.h"
#include "msdf-atlas-gen/types.h"
#include "rurouni/core/scene.hpp"
#include "rurouni/editor/config.h"
#include "rurouni/editor/logger.hpp"
#include "rurouni/editor/state.hpp"
#include "rurouni/editor/ui.hpp"

// rurouni
#include "rurouni/common/logger.hpp"
#include "rurouni/core/asset_manager.hpp"
#include "rurouni/core/logger.hpp"
#include "rurouni/core/module.hpp"
#include "rurouni/error.hpp"
#include "rurouni/event/application_event.hpp"
#include "rurouni/event/event_system.hpp"
#include "rurouni/event/window_event.hpp"
#include "rurouni/graphics/batch_renderer.hpp"
#include "rurouni/graphics/font.hpp"
#include "rurouni/graphics/logger.hpp"
#include "rurouni/graphics/render_api.hpp"
#include "rurouni/graphics/window.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/time.hpp"

// external
#include <imgui/imgui.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/unordered_map.hpp>

// std
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <regex>
#include <utility>

namespace rr::editor {

Editor::Editor(const graphics::WindowSpecification& windowSpec) {
    m_ExecPath = system::get_current_executable_path();
    m_AppName = m_ExecPath.stem();
    m_SharedDataDir = m_ExecPath.parent_path() / RR_EDITOR_RELATIVE_DATA_DIR;
    m_SharedConfigDir =
        m_ExecPath.parent_path() / RR_EDITOR_RELATIVE_CONFIG_DIR;
    m_UserDataDir = system::get_app_user_data_dir(m_AppName);
    m_UserConfigDir = system::get_app_user_config_dir(m_AppName);

    // initializing loggers
    std::string currentDateTime = time::get_current_date_and_time();
    system::Path logPath =
        m_UserDataDir / "logs" / currentDateTime.append(".log");
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_st>());
    sinks.push_back(
        std::make_shared<spdlog::sinks::basic_file_sink_st>(logPath));

    graphics::init_logger(sinks);
    common::init_logger(sinks);
    editor::init_logger(sinks);
    core::init_logger(sinks);

    spdlog::set_level(spdlog::level::level_enum::trace);
    spdlog::flush_on(spdlog::level::level_enum::trace);

    // filepath info
    editor::info("app name ........ : {}", m_AppName);
    editor::info("exec path ....... : {}", m_ExecPath);
    editor::info("exec dir ........ : {}", m_ExecPath.parent_path());
    editor::info("shared data dir . : {}", m_SharedDataDir);
    editor::info("shared config dir : {}", m_SharedConfigDir);
    editor::info("user data dir ... : {}", m_UserDataDir);
    editor::info("user config dir . : {}", m_UserConfigDir);

    m_EventSystem = std::make_shared<event::EventSystem>();
    m_EventSystem->subscribe<event::WindowClose>(this);
    m_EventSystem->subscribe<event::ApplicationClose>(this);
    m_EventSystem->subscribe<event::WindowFramebufferResize>(this);

    m_Window = std::make_shared<graphics::Window>(windowSpec, m_EventSystem);

    m_Renderer = std::make_shared<graphics::BatchRenderer>();

    ui::init(*m_Window, m_SharedDataDir, m_UserDataDir, m_UserConfigDir);

    // read project history
    read_module_history();
}

Editor::~Editor() {
    m_EventSystem->unsubscribe_from_all(this);
    ui::terminate();
}

void Editor::run() {
    std::chrono::high_resolution_clock timer;
    auto last_update = timer.now();

    m_Running = true;
    while (m_Running) {
        auto now = timer.now();
        m_DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(
                          now - last_update)
                          .count() /
                      1000.0f;
        last_update = now;

        update(m_DeltaTime);
        render();
    }
}

void Editor::update(float dt) {
    m_Window->update(dt);
    ui::update(dt, m_UserConfigDir);

    if (m_NextScene != nullptr) {
        m_CurrentScene = std::move(m_NextScene);
        m_NextScene = nullptr;
    }

    if (m_CurrentScene != nullptr) {
        m_CurrentScene->on_update(dt);
    }
}

void Editor::render() {
    graphics::api::set_clear_color({0.05f, 0.05f, 0.05f, 1.0f});
    graphics::api::clear();
    ui::begin();

    if (m_CurrentModule.has_value()) {
        ui::show_dockspace();

        if (m_CurrentScene != nullptr) {
            ui::show_scene_viewport(
                *m_CurrentScene, std::bind(&Editor::draw_scene, this),
                std::bind(&Editor::change_scene, this, std::placeholders::_1));
            ui::show_properties(m_CurrentScene->get_registry(),
                                *m_AssetManager);
            ui::show_scene_manager(
                *m_CurrentScene,
                std::bind(&Editor::change_scene, this, std::placeholders::_1),
                std::bind(&Editor::save_scene, this, std::placeholders::_1));
        }

        ui::show_asset_manager(
            *m_AssetManager,
            std::bind(&Editor::asset_import_texture, this,
                      std::placeholders::_1),
            std::bind(&Editor::asset_import_sprites, this,
                      std::placeholders::_1),
            std::bind(&Editor::asset_import_shader, this,
                      std::placeholders::_1),
            std::bind(&Editor::asset_import_font, this, std::placeholders::_1,
                      std::placeholders::_2));
        ui::show_file_browser(m_CurrentModule->get_root_path());

        ui::show_imgui_demo();
    } else {
        ui::show_startup_splash(
            m_ModuleHistory,
            std::bind(&Editor::import_module, this, std::placeholders::_1),
            std::bind(&Editor::open_module, this, std::placeholders::_1),
            std::bind(&Editor::create_module, this, std::placeholders::_1,
                      std::placeholders::_2),
            std::bind(&Editor::exit_application, this));
    }

    ui::show_toasts();

    ui::end();

    m_Window->swap_buffers();
}

void Editor::on_event(std::shared_ptr<event::Event> event) {
    event::dispatch<event::WindowClose>(
        event,
        std::bind(&Editor::on_window_close_event, this, std::placeholders::_1));
    event::dispatch<event::ApplicationClose>(
        event, std::bind(&Editor::on_application_close_event, this,
                         std::placeholders::_1));
    event::dispatch<event::WindowFramebufferResize>(
        event, std::bind(&Editor::on_window_framebuffer_resize, this,
                         std::placeholders::_1));
}

void Editor::on_window_framebuffer_resize(
    std::shared_ptr<event::WindowFramebufferResize> event) {
    // update imgui display size on window-framebuffer resize
    ui::set_display_size(event->get_new_size());
}

void Editor::on_window_close_event(std::shared_ptr<event::WindowClose> event) {
    m_Running = false;
}

void Editor::on_application_close_event(
    std::shared_ptr<event::ApplicationClose> event) {
    m_Running = false;
}

bool Editor::create_module(const system::Path& path, const std::string& name) {
    if (name.empty()) {
        error("module name is empty!");

        ui::Toast toast(ui::ToastType::Error, "failed creating module",
                        "module name is empty!");
        ui::push_toast(toast);

        return false;
    }

    if (!system::exists(path)) {
        error("filepath to selected module destination does not exist!");
        error("path: {}", path);

        ui::Toast toast(
            ui::ToastType::Error, "failed creating module",
            "filepath to selected module destination does not exist!\npath: {}",
            path);
        ui::push_toast(toast);

        return false;
    }

    if (!system::is_directory(path)) {
        error("filepath to selected module destination is not a directory!");
        error("path: {}", path);

        ui::Toast toast(ui::ToastType::Error, "failed creating module",
                        "filepath to selected module destination is not a "
                        "directory!\npath: {}",
                        path);
        ui::push_toast(toast);

        return false;
    }

    if (system::exists(path / name)) {
        error("module path already exists!");
        error("path: {}", path / name);

        ui::Toast toast(ui::ToastType::Error, "failed creating module",
                        "module path already exists!\npath: {}", path / name);
        ui::push_toast(toast);

        return false;
    }

    system::Path modulePath = path / name / "module.json";
    UUID id = UUID::create();

    system::copy(m_SharedDataDir / "module-template", modulePath.parent_path(),
                 system::copy_options::recursive);

    // configure project.json
    std::ifstream in(modulePath);
    std::stringstream buffer;
    buffer << in.rdbuf();
    in.close();

    // TODO this works, but is shit
    std::regex name_regex("RR_MODULE_NAME");
    std::string result = std::regex_replace(buffer.str(), name_regex, name);
    std::regex id_regex("RR_MODULE_ID");
    result = std::regex_replace(result, id_regex, id.to_string());

    debug("\n{}", result);

    std::ofstream out(modulePath);
    out << result;
    out.close();

    // update history in memory
    ModuleHistoryItem item;
    item.Name = name;
    item.RootPath = modulePath.parent_path();
    m_ModuleHistory[id] = item;

    // write memory to file
    return write_module_history();
}

bool Editor::import_module(const system::Path& path) {
    if (!system::exists(path)) {
        error("filepath to selected module does not exist!");
        error("path: {}", path);

        ui::Toast toast(ui::ToastType::Error, "failed importing module",
                        "filepath to selected module does not exist! path: {}",
                        path);
        ui::push_toast(toast);

        return false;
    }

    std::string name;
    UUID id;
    std::ifstream is(path);
    try {
        cereal::JSONInputArchive in(is);
        in(cereal::make_nvp("name", name), cereal::make_nvp("id", id));
    } catch (cereal::Exception& e) {
        error("cereal: {}", e.what());
        error("path: {}", path);

        ui::Toast toast(ui::ToastType::Error, "failed deserialization",
                        "error while reading '{}'. what: {}", path, e.what());
        ui::push_toast(toast);

        return false;
    }
    is.close();

    ModuleHistoryItem item;
    item.Name = name;
    item.RootPath = path.parent_path();
    m_ModuleHistory[id] = item;

    // write memory to file
    return write_module_history();
}

bool Editor::open_module(const UUID& id) {
    if (m_ModuleHistory.find(id) == m_ModuleHistory.end()) {
        error("requested module id is not registered in module history. id: {}",
              id);

        ui::Toast toast(
            ui::ToastType::Error, "failed opening module",
            "requested module id is not registered in module history. id: {}",
            id);
        ui::push_toast(toast);

        return false;
    }

    auto& moduleHistoryItem = m_ModuleHistory[id];

    if (!system::exists(moduleHistoryItem.RootPath / "module.json")) {
        error("requested module does not exist on the filesystem. id: {}", id);

        ui::Toast toast(
            ui::ToastType::Error, "failed opening module",
            "requested module does not exist on the filesystem. id: {}", id);
        ui::push_toast(toast);

        return false;
    }

    m_AssetManager =
        std::make_shared<core::AssetManager>(moduleHistoryItem.RootPath);
    m_AssetManager->read_asset_configuration();

    m_CurrentModule.emplace(m_Window, m_Renderer, m_AssetManager,
                            m_EventSystem);
    auto error = m_CurrentModule->load_from_file(moduleHistoryItem.RootPath /
                                                 "module.json");

    if (error.has_value()) {
        ui::Toast toast(
            ui::ToastType::Error, "failed loading module", "path: {}\nerror:{}",
            moduleHistoryItem.RootPath / "module.json", error->Message);
        ui::push_toast(toast);

        m_CurrentModule.reset();
        return false;
    }

    // load start scene
    auto absStartScenePath =
        moduleHistoryItem.RootPath / m_CurrentModule->get_start_scene_path();
    if (!system::exists(absStartScenePath)) {
        editor::error("start scene path does not exist on filesystem. path: {}",
                      absStartScenePath);

        ui::Toast toast(
            ui::ToastType::Error, "failed opening start scene",
            "start scene path does not exist on filesystem. path: {}",
            absStartScenePath);
        ui::push_toast(toast);

        return false;
    }

    auto& uiContext = ui::get_context();
    auto scene = std::make_unique<core::Scene>(uiContext.SceneViewportSize);
    error =
        scene->read_from_file(absStartScenePath, uiContext.SceneViewportSize);

    if (error.has_value()) {
        ui::Toast toast(ui::ToastType::Error, "failed loading start scene",
                        "path: {}\nerror:{}", absStartScenePath,
                        error->Message);
        ui::push_toast(toast);

        return false;
    }

    m_CurrentScene = std::move(scene);

    return true;
}

bool Editor::read_module_history() {
    system::Path historyPath = m_UserConfigDir / "module_history.json";

    if (system::exists(historyPath)) {
        try {
            std::ifstream is(historyPath);
            cereal::JSONInputArchive in(is);
            in(m_ModuleHistory);
            is.close();
        } catch (cereal::Exception& e) {
            error("cereal: {}", e.what());
            error("path: {}", historyPath.string());

            ui::Toast toast(ui::ToastType::Error, "failed deserialization",
                            "error while reading '{}'. what: {}", historyPath,
                            e.what());
            ui::push_toast(toast);

            return false;
        }

        return cleanup_module_history();
    }

    return true;
}

bool Editor::write_module_history() {
    system::Path historyPath = m_UserConfigDir / "module_history.json";

    if (!system::exists(m_UserConfigDir))
        system::create_directories(m_UserConfigDir);

    try {
        std::ofstream os(historyPath);
        cereal::JSONOutputArchive out(os);
        out(m_ModuleHistory);
    } catch (cereal::Exception& e) {
        error("cereal: {}", e.what());
        error("path: {}", historyPath.string());

        ui::Toast toast(ui::ToastType::Error, "failed serialization",
                        "error while writing '{}'. what: {}", historyPath,
                        e.what());
        ui::push_toast(toast);

        return false;
    }

    return true;
}

bool Editor::cleanup_module_history() {
    std::vector<ModuleHistoryItem> erasedItems;
    for (auto it = m_ModuleHistory.begin(); it != m_ModuleHistory.end();) {
        system::Path moduleRootPath = it->second.RootPath;

        if (!system::exists(moduleRootPath)) {
            erasedItems.push_back(it->second);
            it = m_ModuleHistory.erase(it);
        } else {
            it++;
        }
    }

    if (!erasedItems.empty()) {
        bool success = write_module_history();

        if (success) {
            ui::Toast toast(ui::ToastType::Info, "",
                            "cleaned {} items from history list",
                            erasedItems.size());
            ui::push_toast(toast);
        }

        return success;
    }

    return true;
}

bool Editor::change_scene(const system::Path& path) {
    auto& uiContext = ui::get_context();
    auto scene = std::make_unique<core::Scene>(uiContext.SceneViewportSize);
    auto error = scene->read_from_file(path, uiContext.SceneViewportSize);

    if (error.has_value()) {
        ui::Toast toast(ui::ToastType::Error, "failed deserialization",
                        "path: {}\nerror: {}", path, error->Message);
        ui::push_toast(toast);

        return false;
    }

    m_NextScene = std::move(scene);

    return true;
}

bool Editor::save_scene(const system::Path& path) {
    if (path.empty()) {
        auto error =
            m_CurrentScene->write_to_file(m_CurrentScene->get_filepath());

        if (error.has_value()) {
            ui::Toast toast(ui::ToastType::Error, "failed serialization",
                            "path: {}\nerror: {}",
                            m_CurrentScene->get_filepath(), error->Message);
            ui::push_toast(toast);

            return false;
        }

        return true;
    }

    if (system::exists(path)) {
        warn("path already exists and will be overwritten. path: {}", path);

        ui::Toast toast(ui::ToastType::Warning, "save scene",
                        "path already exists and will be overwritten. path: {}",
                        path);
        ui::push_toast(toast);
    }
    // TODO check not a directory
    auto error = m_CurrentScene->write_to_file(path);

    if (error.has_value()) {
        ui::Toast toast(ui::ToastType::Error, "failed serialization",
                        "path: {}\nerror: {}", path, error->Message);
        ui::push_toast(toast);

        return false;
    }

    return true;
}

void Editor::draw_scene() {
    if (m_CurrentScene != nullptr) {
        m_CurrentScene->on_render(*m_Renderer, *m_AssetManager);
    }
}

bool Editor::asset_import_texture(core::TextureSpecification& spec) {
    if (spec.Filepath.empty()) {
        error("texture import");
        error("empty texture import path");

        ui::Toast toast(ui::ToastType::Error, "texture import",
                        "passed empty texture path");
        ui::push_toast(toast);

        return false;
    }

    if (!system::exists(spec.Filepath)) {
        error("texture import");
        error("file not found: {}", spec.Filepath);

        ui::Toast toast(
            ui::ToastType::Error, "texture import",
            "the requested file to import was not found at path: {}",
            spec.Filepath);
        ui::push_toast(toast);

        return false;
    }

    if (!system::is_regular_file(spec.Filepath)) {
        error("texture import");
        error("not a file: {}", spec.Filepath);

        ui::Toast toast(ui::ToastType::Error, "texture import",
                        "the requested file to import is not a file. path: {}",
                        spec.Filepath);
        ui::push_toast(toast);

        return false;
    }

    if (spec.Filepath.parent_path() !=
        m_CurrentModule->get_root_path() / "textures") {
        // import texture into assets dir
        system::Path texturePath = spec.Filepath;
        system::Path importPath = m_CurrentModule->get_root_path() /
                                  "textures" / spec.Filepath.filename();

        if (!system::exists(importPath)) {
            system::copy(texturePath, importPath);
        }

        spec.Filepath = importPath;
    }

    spec.Id.generate();
    m_AssetManager->get_texture_registry()[spec.Id] = spec;
    // m_AssetManager->write_asconfiguration(m_CurrentModule->get_root_path()
    // / "assets.json");
    m_AssetManager->write_asset_configuration();

    return true;
}

bool Editor::asset_import_sprites(
    std::unordered_map<int, core::SpriteSpecification>& specs) {
    if (specs.empty()) {
        ui::Toast toast(ui::ToastType::Error, "",
                        "import sprite pack was empty");
        ui::push_toast(toast);

        return false;
    }

    auto& spriteSpecs = m_AssetManager->get_sprite_registry();

    for (auto& [idx, spec] : specs) {
        spriteSpecs[spec.Id] = spec;
    }

    auto error = m_AssetManager->write_asset_configuration();

    if (error.has_value()) {
        ui::Toast toast(ui::ToastType::Error, "failed serialization",
                        "error: {}", error->Message);
        ui::push_toast(toast);

        return false;
    }

    return true;
}

bool Editor::asset_import_shader(core::ShaderSpecification& spec) {
    if (spec.VertexSourcePath.empty() || spec.FragmentSourcePath.empty()) {
        error("shader import");
        error("empty shader import path");

        ui::Toast toast(ui::ToastType::Error, "shader import",
                        "passed empty shader path");
        ui::push_toast(toast);

        return false;
    }

    if (!system::exists(spec.VertexSourcePath)) {
        error("shader import");
        error("file not found: {}", spec.VertexSourcePath);

        ui::Toast toast(
            ui::ToastType::Error, "shader import",
            "the requested vertex shader to import was not found at path: {}",
            spec.VertexSourcePath);
        ui::push_toast(toast);

        return false;
    }

    if (!system::exists(spec.FragmentSourcePath)) {
        error("shader import");
        error("file not found: {}", spec.FragmentSourcePath);

        ui::Toast toast(
            ui::ToastType::Error, "shader import",
            "the requested fragment shader to import was not found at path: {}",
            spec.FragmentSourcePath);
        ui::push_toast(toast);

        return false;
    }

    if (!system::is_regular_file(spec.VertexSourcePath)) {
        error("shader import");
        error("not a file: {}", spec.VertexSourcePath);

        ui::Toast toast(
            ui::ToastType::Error, "shader import",
            "the requested vertex shader to import is not a file. path: {}",
            spec.VertexSourcePath);
        ui::push_toast(toast);

        return false;
    }

    if (!system::is_regular_file(spec.FragmentSourcePath)) {
        error("shader import");
        error("not a file: {}", spec.FragmentSourcePath);

        ui::Toast toast(
            ui::ToastType::Error, "shader import",
            "the requested fragment shader to import is not a file. path: {}",
            spec.FragmentSourcePath);
        ui::push_toast(toast);

        return false;
    }

    if (spec.VertexSourcePath.parent_path() !=
        m_CurrentModule->get_root_path() / "shaders") {
        // import texture into assets dir
        system::Path shaderPath = spec.VertexSourcePath;
        system::Path importPath = m_CurrentModule->get_root_path() / "shaders" /
                                  spec.VertexSourcePath.filename();

        if (!system::exists(importPath)) {
            system::copy(shaderPath, importPath);
        }

        spec.VertexSourcePath = importPath;
    }

    if (spec.FragmentSourcePath.parent_path() !=
        m_CurrentModule->get_root_path() / "shaders") {
        // import texture into assets dir
        system::Path shaderPath = spec.FragmentSourcePath;
        system::Path importPath = m_CurrentModule->get_root_path() / "shaders" /
                                  spec.FragmentSourcePath.filename();

        if (!system::exists(importPath)) {
            system::copy(shaderPath, importPath);
        }

        spec.FragmentSourcePath = importPath;
    }

    spec.Id.generate();
    m_AssetManager->get_shader_registry()[spec.Id] = spec;
    auto error = m_AssetManager->write_asset_configuration();

    if (error.has_value()) {
        ui::Toast toast(ui::ToastType::Error, "failed serialization",
                        "error: {}", error->Message);
        ui::push_toast(toast);

        return false;
    }

    return true;
}

#include <msdf-atlas-gen/msdf-atlas-gen.h>

static msdfgen::FontHandle* loadVarFont(msdfgen::FreetypeHandle* library,
                                        const char* filename) {
    std::string buffer;
    while (*filename && *filename != '?')
        buffer.push_back(*filename++);
    msdfgen::FontHandle* font = msdfgen::loadFont(library, buffer.c_str());
    if (font && *filename++ == '?') {
        do {
            buffer.clear();
            while (*filename && *filename != '=')
                buffer.push_back(*filename++);
            if (*filename == '=') {
                double value = 0;
                int skip = 0;
                if (sscanf(++filename, "%lf%n", &value, &skip) == 1) {
                    msdfgen::setFontVariationAxis(library, font, buffer.c_str(),
                                                  value);
                    filename += skip;
                }
            }
        } while (*filename++ == '&');
    }
    return font;
}

template <typename T,
          typename S,
          int N,
          msdf_atlas::GeneratorFunction<S, N> GEN_FN>
static bool makeAtlas(const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
                      const std::vector<msdf_atlas::FontGeometry>& fonts,
                      const ui::FontImportConfiguration& config) {
    msdf_atlas::ImmediateAtlasGenerator<S, N, GEN_FN,
                                        msdf_atlas::BitmapAtlasStorage<T, N> >
        generator(config.Width, config.Height);
    generator.setAttributes(config.GeneratorAttributes);
    generator.setThreadCount(config.ThreadCount);
    generator.generate(glyphs.data(), glyphs.size());
    msdfgen::BitmapConstRef<T, N> bitmap =
        (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

    bool success = true;

    if (!config.ImageFilepath.empty()) {
        if (msdf_atlas::saveImage(bitmap, config.ImageFormat,
                                  config.ImageFilepath.c_str(),
                                  config.YDirection))
            fputs("Atlas image file saved.\n", stderr);
        else {
            success = false;
            fputs("Failed to save the atlas as an image file.\n", stderr);
        }
    }

    // #ifndef MSDF_ATLAS_NO_ARTERY_FONT
    //     if (config.arteryFontFilename) {
    //         ArteryFontExportProperties arfontProps;
    //         arfontProps.fontSize = config.emSize;
    //         arfontProps.pxRange = config.pxRange;
    //         arfontProps.imageType = config.imageType;
    //         arfontProps.imageFormat = config.imageFormat;
    //         arfontProps.yDirection = config.yDirection;
    //         if (exportArteryFont<float>(fonts.data(), fonts.size(), bitmap,
    //         config.arteryFontFilename, arfontProps))
    //             fputs("Artery Font file generated.\n", stderr);
    //         else {
    //             success = false;
    //             fputs("Failed to generate Artery Font file.\n", stderr);
    //         }
    //     }
    // #endif

    return success;
}

bool Editor::asset_import_font(ui::FontImportSpecification& fontInput,
                               ui::FontImportConfiguration& config) {
    core::FontSpecification fontSpec;

    int fixedWidth = -1, fixedHeight = -1;
    double minEmSize = 0;
    const char* imageFormatName = nullptr;
    config.GeneratorAttributes.config.overlapSupport =
        !config.PreprocessGeometry;
    config.GeneratorAttributes.scanlinePass = !config.PreprocessGeometry;
    msdf_atlas::TightAtlasPacker::DimensionsConstraint atlasSizeConstraint =
        msdf_atlas::TightAtlasPacker::DimensionsConstraint::
            MULTIPLE_OF_FOUR_SQUARE;

    enum {
        /// Range specified in EMs
        RANGE_EM,
        /// Range specified in output pixels
        RANGE_PIXEL,
    } rangeMode = RANGE_PIXEL;
    double rangeValue = 0;

    if (fontInput.FontFilepath.empty()) {
        error("import font -> No font specified.");
    ui:
        ui::push_toast(
            ui::Toast(ui::ToastType::Error, "", "No font specified."));
        return false;
    }

    // TODO handle empty output path
    // if (config.ImageFilepath.empty()) {
    //     error("No output specified.\n", stderr);
    //     return 0;
    // }
    // bool layoutOnly = !(config.arteryFontFilename || config.imageFilename);
    bool layoutOnly = false;

    // Fix up configuration based on related values
    if (!(config.ImageType == msdf_atlas::ImageType::PSDF ||
          config.ImageType == msdf_atlas::ImageType::MSDF ||
          config.ImageType == msdf_atlas::ImageType::MTSDF))
        config.MiterLimit = 0;
    if (config.EmSize > minEmSize)
        minEmSize = config.EmSize;
    if (!(fixedWidth > 0 && fixedHeight > 0) && !(minEmSize > 0)) {
        fputs("Neither atlas size nor glyph size selected, using default...\n",
              stderr);
        minEmSize = MSDF_ATLAS_DEFAULT_EM_SIZE;
    }
    if (config.ImageType == msdf_atlas::ImageType::HARD_MASK ||
        config.ImageType == msdf_atlas::ImageType::SOFT_MASK) {
        rangeMode = RANGE_PIXEL;
        rangeValue = 1;
    } else if (rangeValue <= 0) {
        rangeMode = RANGE_PIXEL;
        rangeValue = DEFAULT_PIXEL_RANGE;
    }
    // We always export json
    // if (config.Kerning && !(config.arteryFontFilename || config.jsonFilename
    // || config.shadronPreviewFilename))
    //     config.Kerning = false;
    if (config.ThreadCount <= 0)
        config.ThreadCount =
            std::max((int)std::thread::hardware_concurrency(), 1);
    if (config.GeneratorAttributes.scanlinePass) {
        // TODO error corection mode
        // if (explicitErrorCorrectionMode &&
        // config.GeneratorAttributes.config.errorCorrection.distanceCheckMode
        // != msdfgen::ErrorCorrectionConfig::DO_NOT_CHECK_DISTANCE) {
        //     const char *fallbackModeName = "unknown";
        //     switch (config.generatorAttributes.config.errorCorrection.mode) {
        //         case msdfgen::ErrorCorrectionConfig::DISABLED:
        //         fallbackModeName = "disabled"; break; case
        //         msdfgen::ErrorCorrectionConfig::INDISCRIMINATE:
        //         fallbackModeName = "distance-fast"; break; case
        //         msdfgen::ErrorCorrectionConfig::EDGE_PRIORITY:
        //         fallbackModeName = "auto-fast"; break; case
        //         msdfgen::ErrorCorrectionConfig::EDGE_ONLY: fallbackModeName =
        //         "edge-fast"; break;
        //     }
        //     fprintf(stderr, "Selected error correction mode not compatible
        //     with scanline mode, falling back to %s.\n", fallbackModeName);
        // }
        config.GeneratorAttributes.config.errorCorrection.distanceCheckMode =
            msdfgen::ErrorCorrectionConfig::DO_NOT_CHECK_DISTANCE;
    }

    // Finalize image format
    msdf_atlas::ImageFormat imageExtension =
        msdf_atlas::ImageFormat::UNSPECIFIED;
    if (!config.ImageFilepath.empty()) {
        if (config.ImageFilepath.extension() == ".png")
            imageExtension = msdf_atlas::ImageFormat::PNG;
        else if (config.ImageFilepath.extension() == ".bmp")
            imageExtension = msdf_atlas::ImageFormat::BMP;
        else if (config.ImageFilepath.extension() == ".tif" ||
                 config.ImageFilepath.extension() == ".tiff")
            imageExtension = msdf_atlas::ImageFormat::TIFF;
        else if (config.ImageFilepath.extension() == ".txt")
            imageExtension = msdf_atlas::ImageFormat::TEXT;
        else if (config.ImageFilepath.extension() == ".bin")
            imageExtension = msdf_atlas::ImageFormat::BINARY;
    }
    if (config.ImageFormat == msdf_atlas::ImageFormat::UNSPECIFIED) {
        config.ImageFormat = msdf_atlas::ImageFormat::PNG;
        imageFormatName = "png";
        // If image format is not specified and -imageout is the only image
        // output, infer format from its extension
        if (imageExtension != msdf_atlas::ImageFormat::UNSPECIFIED
            // && !config.arteryFontFilename
        )
            config.ImageFormat = imageExtension;
    }
    if (config.ImageType == msdf_atlas::ImageType::MTSDF &&
        config.ImageFormat == msdf_atlas::ImageFormat::BMP)
        require(false,
                "Atlas type not compatible with image format. MTSDF requires a "
                "format with alpha channel.");
    // #ifndef MSDF_ATLAS_NO_ARTERY_FONT
    //     if (config.arteryFontFilename && !(config.imageFormat ==
    //     ImageFormat::PNG || config.imageFormat == ImageFormat::BINARY ||
    //     config.imageFormat == ImageFormat::BINARY_FLOAT)) {
    //         config.arteryFontFilename = nullptr;
    //         result = 1;
    //         fputs("Error: Unable to create an Artery Font file with the
    //         specified image format!\n", stderr);
    //         // Recheck whether there is anything else to do
    //         if (!(config.arteryFontFilename || config.imageFilename ||
    //         config.jsonFilename || config.csvFilename ||
    //         config.shadronPreviewFilename))
    //             return result;
    //         layoutOnly = !(config.arteryFontFilename ||
    //         config.imageFilename);
    //     }
    // #endif
    if (imageExtension != msdf_atlas::ImageFormat::UNSPECIFIED) {
        // Warn if image format mismatches -imageout extension
        bool mismatch = false;
        switch (config.ImageFormat) {
            case msdf_atlas::ImageFormat::TEXT:
            case msdf_atlas::ImageFormat::TEXT_FLOAT:
                mismatch = imageExtension != msdf_atlas::ImageFormat::TEXT;
                break;
            case msdf_atlas::ImageFormat::BINARY:
            case msdf_atlas::ImageFormat::BINARY_FLOAT:
            case msdf_atlas::ImageFormat::BINARY_FLOAT_BE:
                mismatch = imageExtension != msdf_atlas::ImageFormat::BINARY;
                break;
            default:
                mismatch = imageExtension != config.ImageFormat;
        }
        if (mismatch)
            error(
                "Warning: Output image file extension does not match the "
                "image's actual format ({})!",
                imageFormatName);
    }
    imageFormatName = nullptr;  // No longer consistent with imageFormat
    bool floatingPointFormat =
        (config.ImageFormat == msdf_atlas::ImageFormat::TIFF ||
         config.ImageFormat == msdf_atlas::ImageFormat::TEXT_FLOAT ||
         config.ImageFormat == msdf_atlas::ImageFormat::BINARY_FLOAT ||
         config.ImageFormat == msdf_atlas::ImageFormat::BINARY_FLOAT_BE);

    // Load fonts
    std::vector<msdf_atlas::GlyphGeometry> glyphs;
    std::vector<msdf_atlas::FontGeometry> fonts;
    bool anyCodepointsAvailable = false;
    {
        class FontHolder {
            msdfgen::FreetypeHandle* ft;
            msdfgen::FontHandle* font;
            const char* fontFilename;

           public:
            FontHolder()
                : ft(msdfgen::initializeFreetype()),
                  font(nullptr),
                  fontFilename(nullptr) {}
            ~FontHolder() {
                if (ft) {
                    if (font)
                        msdfgen::destroyFont(font);
                    msdfgen::deinitializeFreetype(ft);
                }
            }
            bool load(const char* fontFilename, bool isVarFont) {
                if (ft && fontFilename) {
                    if (this->fontFilename &&
                        !strcmp(this->fontFilename, fontFilename))
                        return true;
                    if (font)
                        msdfgen::destroyFont(font);
                    if ((font = isVarFont
                                    ? loadVarFont(ft, fontFilename)
                                    : msdfgen::loadFont(ft, fontFilename))) {
                        this->fontFilename = fontFilename;
                        return true;
                    }
                    this->fontFilename = nullptr;
                }
                return false;
            }
            operator msdfgen::FontHandle*() const { return font; }
        } font;

        // for (FontInput &fontInput : fontInputs) {
        if (!font.load(fontInput.FontFilepath.c_str(), fontInput.VariableFont))
            require(false, "Failed to load specified font file.");
        if (fontInput.FontScale <= 0)
            fontInput.FontScale = 1;

        // Load character set
        msdf_atlas::Charset charset;
        if (!fontInput.CharsetFilepath.empty()) {
            if (!charset.load(
                    fontInput.CharsetFilepath.c_str(),
                    fontInput.GlyphIdentifierType !=
                        msdf_atlas::GlyphIdentifierType::UNICODE_CODEPOINT))
                require(false,
                        fontInput.GlyphIdentifierType ==
                                msdf_atlas::GlyphIdentifierType::GLYPH_INDEX
                            ? "Failed to load glyph set specification."
                            : "Failed to load character set specification.");
        } else if (!config.Charset.empty()) {
            for (auto& codepoint : config.Charset) {
                charset.add(codepoint);
            }
        } else {
            charset = msdf_atlas::Charset::ASCII;
            fontInput.GlyphIdentifierType =
                msdf_atlas::GlyphIdentifierType::UNICODE_CODEPOINT;
        }

        // Load glyphs
        msdf_atlas::FontGeometry fontGeometry(&glyphs);
        int glyphsLoaded = -1;
        switch (fontInput.GlyphIdentifierType) {
            case msdf_atlas::GlyphIdentifierType::GLYPH_INDEX:
                glyphsLoaded = fontGeometry.loadGlyphset(
                    font, fontInput.FontScale, charset,
                    config.PreprocessGeometry, config.Kerning);
                break;
            case msdf_atlas::GlyphIdentifierType::UNICODE_CODEPOINT:
                glyphsLoaded = fontGeometry.loadCharset(
                    font, fontInput.FontScale, charset,
                    config.PreprocessGeometry, config.Kerning);
                anyCodepointsAvailable |= glyphsLoaded > 0;
                break;
        }
        if (glyphsLoaded < 0)
            require(false, "Failed to load glyphs from font.");
        info("Loaded geometry of {} out of {} glyphs.", glyphsLoaded,
             (int)charset.size());
        // List missing glyphs
        if (glyphsLoaded < (int)charset.size()) {
            fprintf(stderr, "Missing %d %s", (int)charset.size() - glyphsLoaded,
                    fontInput.GlyphIdentifierType ==
                            msdf_atlas::GlyphIdentifierType::UNICODE_CODEPOINT
                        ? "codepoints"
                        : "glyphs");
            bool first = true;
            switch (fontInput.GlyphIdentifierType) {
                case msdf_atlas::GlyphIdentifierType::GLYPH_INDEX:
                    for (graphics::unicode_t cp : charset)
                        if (!fontGeometry.getGlyph(msdfgen::GlyphIndex(cp)))
                            fprintf(stderr, "%c 0x%02X",
                                    first ? ((first = false), ':') : ',', cp);
                    break;
                case msdf_atlas::GlyphIdentifierType::UNICODE_CODEPOINT:
                    for (graphics::unicode_t cp : charset)
                        if (!fontGeometry.getGlyph(cp))
                            fprintf(stderr, "%c 0x%02X",
                                    first ? ((first = false), ':') : ',', cp);
                    break;
            }
            fprintf(stderr, "\n");
        }

        if (fontInput.FontName.c_str())
            fontGeometry.setName(fontInput.FontName.c_str());

        fonts.push_back((msdf_atlas::FontGeometry &&) fontGeometry);
    }
    // }
    if (glyphs.empty())
        require(false, "No glyphs loaded.");

    // Determine final atlas dimensions, scale and range, pack glyphs
    {
        double unitRange = 0, pxRange = 0;
        switch (rangeMode) {
            case RANGE_EM:
                unitRange = rangeValue;
                break;
            case RANGE_PIXEL:
                pxRange = rangeValue;
                break;
        }
        bool fixedDimensions = fixedWidth >= 0 && fixedHeight >= 0;
        bool fixedScale = config.EmSize > 0;
        msdf_atlas::TightAtlasPacker atlasPacker;
        if (fixedDimensions)
            atlasPacker.setDimensions(fixedWidth, fixedHeight);
        else
            atlasPacker.setDimensionsConstraint(atlasSizeConstraint);
        atlasPacker.setPadding(
            config.ImageType == msdf_atlas::ImageType::MSDF ||
                    config.ImageType == msdf_atlas::ImageType::MTSDF
                ? 0
                : -1);
        // TODO: In this case (if padding is -1), the border pixels of each
        // glyph are black, but still computed. For floating-point output, this
        // may play a role.
        if (fixedScale)
            atlasPacker.setScale(config.EmSize);
        else
            atlasPacker.setMinimumScale(minEmSize);
        atlasPacker.setPixelRange(pxRange);
        atlasPacker.setUnitRange(unitRange);
        atlasPacker.setMiterLimit(config.MiterLimit);
        if (int remaining = atlasPacker.pack(glyphs.data(), glyphs.size())) {
            if (remaining < 0) {
                require(false, "Failed to pack glyphs into atlas.");
            } else {
                error(
                    "Error: Could not fit {} out of {} glyphs into the atlas.",
                    remaining, glyphs.size());
                return 1;
            }
        }
        atlasPacker.getDimensions(config.Width, config.Height);
        if (!(config.Width > 0 && config.Height > 0))
            require(false, "Unable to determine atlas size.");
        config.EmSize = atlasPacker.getScale();
        config.PxRange = atlasPacker.getPixelRange();
        if (!fixedScale)
            debug("Glyph size: {} pixels/EM", config.EmSize);
        if (!fixedDimensions)
            debug("Atlas dimensions: {} x {}", config.Width, config.Height);
    }

    // Generate atlas bitmap
    if (!layoutOnly) {
        // Edge coloring
        if (config.ImageType == msdf_atlas::ImageType::MSDF ||
            config.ImageType == msdf_atlas::ImageType::MTSDF) {
            if (config.ExpensiveColoring) {
                msdf_atlas::Workload(
                    [&glyphs, &config](int i, int threadNo) -> bool {
                        unsigned long long glyphSeed =
                            (LCG_MULTIPLIER * (config.ColoringSeed ^ i) +
                             LCG_INCREMENT) *
                            !!config.ColoringSeed;
                        glyphs[i].edgeColoring(config.EdgeColoring,
                                               config.AngleThreshold,
                                               glyphSeed);
                        return true;
                    },
                    glyphs.size())
                    .finish(config.ThreadCount);
            } else {
                unsigned long long glyphSeed = config.ColoringSeed;
                for (msdf_atlas::GlyphGeometry& glyph : glyphs) {
                    glyphSeed *= LCG_MULTIPLIER;
                    glyph.edgeColoring(config.EdgeColoring,
                                       config.AngleThreshold, glyphSeed);
                }
            }
        }

        bool success = false;
        switch (config.ImageType) {
            case msdf_atlas::ImageType::HARD_MASK:
                if (floatingPointFormat)
                    success = makeAtlas<float, float, 1,
                                        msdf_atlas::scanlineGenerator>(
                        glyphs, fonts, config);
                else
                    success = makeAtlas<msdf_atlas::byte, float, 1,
                                        msdf_atlas::scanlineGenerator>(
                        glyphs, fonts, config);
                break;
            case msdf_atlas::ImageType::SOFT_MASK:
            case msdf_atlas::ImageType::SDF:
                if (floatingPointFormat)
                    success =
                        makeAtlas<float, float, 1, msdf_atlas::sdfGenerator>(
                            glyphs, fonts, config);
                else
                    success = makeAtlas<msdf_atlas::byte, float, 1,
                                        msdf_atlas::sdfGenerator>(glyphs, fonts,
                                                                  config);
                break;
            case msdf_atlas::ImageType::PSDF:
                if (floatingPointFormat)
                    success =
                        makeAtlas<float, float, 1, msdf_atlas::psdfGenerator>(
                            glyphs, fonts, config);
                else
                    success = makeAtlas<msdf_atlas::byte, float, 1,
                                        msdf_atlas::psdfGenerator>(
                        glyphs, fonts, config);
                break;
            case msdf_atlas::ImageType::MSDF:
                if (floatingPointFormat)
                    success =
                        makeAtlas<float, float, 3, msdf_atlas::msdfGenerator>(
                            glyphs, fonts, config);
                else
                    success = makeAtlas<msdf_atlas::byte, float, 3,
                                        msdf_atlas::msdfGenerator>(
                        glyphs, fonts, config);
                break;
            case msdf_atlas::ImageType::MTSDF:
                if (floatingPointFormat)
                    success =
                        makeAtlas<float, float, 4, msdf_atlas::mtsdfGenerator>(
                            glyphs, fonts, config);
                else
                    success = makeAtlas<msdf_atlas::byte, float, 4,
                                        msdf_atlas::mtsdfGenerator>(
                        glyphs, fonts, config);
                break;
        }
        if (!success)
            // result = 1;
            return false;
    }

    // TODO export json
    // if (config.jsonFilename) {
    //     if (exportJSON(fonts.data(), fonts.size(), config.emSize,
    //     config.pxRange, config.width, config.height, config.imageType,
    //     config.yDirection, config.jsonFilename, config.kerning))
    //         fputs("Glyph layout and metadata written into JSON file.\n",
    //         stderr);
    //     else {
    //         result = 1;
    //         fputs("Failed to write JSON output file.\n", stderr);
    //     }
    // }

    core::FontSpecification spec;
    spec.Id.generate();
    spec.Name = fontInput.FontName;
    spec.Filepath = config.ImageFilepath;

    const msdfgen::FontMetrics& metrics = fonts.front().getMetrics();
    spec.FontMetrics.emSize = metrics.emSize;
    spec.FontMetrics.lineHeight = metrics.lineHeight;
    spec.FontMetrics.ascender = metrics.ascenderY;
    spec.FontMetrics.descender = metrics.descenderY;
    spec.FontMetrics.underlineY = metrics.underlineY;
    spec.FontMetrics.underlineThickness = metrics.underlineThickness;

    graphics::FontAtlasType type;
    std::string msdfTypeString =
        std::string(magic_enum::enum_name(config.ImageType));
    type = magic_enum::enum_cast<graphics::FontAtlasType>(
               msdfTypeString, magic_enum::case_insensitive)
               .value();
    spec.AtlasMetrics.type = type;
    spec.AtlasMetrics.size = config.EmSize;
    spec.AtlasMetrics.width = config.Width;
    spec.AtlasMetrics.height = config.Height;
    if (config.YDirection == msdf_atlas::YDirection::BOTTOM_UP) {
        spec.AtlasMetrics.yOrigin = graphics::YOrigin::Bottom;
    } else if (config.YDirection == msdf_atlas::YDirection::TOP_DOWN) {
        spec.AtlasMetrics.yOrigin = graphics::YOrigin::Top;
    }

    for (const msdf_atlas::GlyphGeometry& glyph : fonts.front().getGlyphs()) {
        graphics::GlyphMetrics metrics;
        metrics.codepoint = glyph.getCodepoint();
        metrics.advance = glyph.getAdvance();

        // TODO this is bottom up. top bottom switch is missing
        double l, b, r, t;
        glyph.getQuadAtlasBounds(l, b, r, t);
        metrics.atlasBounds.left = l;
        metrics.atlasBounds.bottom = b;
        metrics.atlasBounds.right = r;
        metrics.atlasBounds.top = t;
        glyph.getQuadPlaneBounds(l, b, r, t);
        metrics.planeBounds.left = l;
        metrics.planeBounds.bottom = b;
        metrics.planeBounds.right = r;
        metrics.planeBounds.top = t;

        spec.GlyphMetrics[glyph.getCodepoint()] = metrics;
    }

    m_AssetManager->get_font_registry()[spec.Id] = spec;
    m_AssetManager->write_asset_configuration();

    return true;
}

void Editor::exit_application() {
    m_Running = false;
}

}  // namespace rr::editor
