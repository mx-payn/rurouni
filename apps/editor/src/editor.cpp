// editor
#include "rurouni/editor.hpp"
#include "rurouni/core/module.hpp"
#include "rurouni/core/scene.hpp"
#include "rurouni/editor/config.h"
#include "rurouni/editor/logger.hpp"
#include "rurouni/editor/state.hpp"
#include "rurouni/editor/ui.hpp"
#include "rurouni/editor/ui_modals/error.hpp"
#include "rurouni/editor/ui_modals/module_create.hpp"
#include "rurouni/editor/ui_panels/startup_splash.hpp"

// rurouni
#include "rurouni/common/logger.hpp"
#include "rurouni/core/logger.hpp"
#include "rurouni/event/application_event.hpp"
#include "rurouni/event/event_system.hpp"
#include "rurouni/event/window_event.hpp"
#include "rurouni/graphics/batch_renderer.hpp"
#include "rurouni/graphics/logger.hpp"
#include "rurouni/graphics/render_api.hpp"
#include "rurouni/graphics/window.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/time.hpp"

// external
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

    // modals
    m_ModuleCreateModal = std::make_unique<ui::ModuleCreateModal>();
    m_ErrorModal = std::make_unique<ui::ErrorModal>();

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

    if (!m_CurrentScenes.empty())
        m_CurrentScenes.back()->on_update(dt);
}

void Editor::render() {
    graphics::api::set_clear_color({0.05f, 0.05f, 0.05f, 1.0f});
    graphics::api::clear();
    ui::begin();

    if (m_CurrentModule.has_value()) {
        ui::draw_dockspace(m_UIState);
    } else {
        ui::StartupSplash::draw(
            m_UIState, *m_EventSystem, m_ModuleHistory,
            std::bind(&Editor::import_module, this, std::placeholders::_1),
            std::bind(&Editor::open_module, this, std::placeholders::_1));
    }

    // draw modals
    m_ErrorModal->draw();
    m_ModuleCreateModal->draw(
        m_UIState, std::bind(&Editor::create_module, this,
                             std::placeholders::_1, std::placeholders::_2));

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

void Editor::create_module(const system::Path& path, const std::string& name) {
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
    write_module_history();
}

void Editor::import_module(const system::Path& path) {
    if (!system::exists(path)) {
        error("filepath to selected module does not exist!");
        error("path: {}", path);

        ui::ErrorModal::push_error(
            "failed importing module",
            "filepath to selected module does not exist! path: {}", path);
        return;
    }

    std::string name;
    UUID id;
    try {
        std::ifstream is(path);
        cereal::JSONInputArchive in(is);
        in(cereal::make_nvp("name", name), cereal::make_nvp("id", id));
        is.close();
    } catch (cereal::Exception& e) {
        error("cereal: {}", e.what());
        error("path: {}", path);

        ui::ErrorModal::push_error("failed deserialization",
                                   "error while reading '{}'. what: {}", path,
                                   e.what());
        return;
    }

    ModuleHistoryItem item;
    item.Name = name;
    item.RootPath = path.parent_path();
    m_ModuleHistory[id] = item;

    // write memory to file
    write_module_history();
}

void Editor::open_module(const UUID& id) {
    if (m_ModuleHistory.find(id) == m_ModuleHistory.end()) {
        error("requested module id is not registered in module history. id: {}",
              id);

        ui::ErrorModal::push_error(
            "failed opening module",
            "requested module id is not registered in module history. id: {}",
            id);
        return;
    }

    auto& moduleHistoryItem = m_ModuleHistory[id];

    if (!system::exists(moduleHistoryItem.RootPath / "module.json")) {
        error("requested module does not exist on the filesystem. id: {}", id);

        ui::ErrorModal::push_error(
            "failed opening module",
            "requested module does not exist on the filesystem. id: {}", id);
        return;
    }

    m_CurrentModule.emplace(m_Window, m_Renderer, m_EventSystem);
    auto error = m_CurrentModule->load_from_file(moduleHistoryItem.RootPath /
                                                 "module.json");

    if (error.has_value()) {
        editor::error("failed opening module. path: {}, error: {}",
                      moduleHistoryItem.RootPath / "module.json",
                      error->Message);
        ui::ErrorModal::push_error("failed opening module", "{}",
                                   error->Message);
        m_CurrentModule.reset();
        return;
    }

    // load start scene
    auto absStartScenePath =
        moduleHistoryItem.RootPath / m_CurrentModule->get_start_scene_path();
    if (!system::exists(absStartScenePath)) {
        editor::error("start scene path does not exist on filesystem. path: {}",
                      absStartScenePath);
        ui::ErrorModal::push_error(
            "failed opening start scene",
            "start scene path does not exist on filesystem. path: {}",
            absStartScenePath);
        return;
    }

    auto scene = std::make_unique<core::Scene>(math::ivec2(1.0f));
    scene->read_from_file(absStartScenePath, m_UIState.SceneViewportSize);
    m_CurrentScenes.push_back(std::move(scene));
}

void Editor::read_module_history() {
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

            ui::ErrorModal::push_error("failed deserialization",
                                       "error while reading '{}'. what: {}",
                                       historyPath, e.what());
        }

        cleanup_module_history();
    }
}

void Editor::write_module_history() {
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

        ui::ErrorModal::push_error("failed serialization",
                                   "error while writing '{}'. what: {}",
                                   historyPath, e.what());
    }
}

void Editor::cleanup_module_history() {
    bool didErase = false;
    for (auto it = m_ModuleHistory.begin(); it != m_ModuleHistory.end();) {
        system::Path moduleRootPath = it->second.RootPath;

        if (!system::exists(moduleRootPath)) {
            it = m_ModuleHistory.erase(it);
            didErase = true;
        } else {
            it++;
        }
    }

    if (didErase)
        write_module_history();
}

}  // namespace rr::editor
