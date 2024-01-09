#include "rurouni/editor/ui.hpp"

#include "imgui/imgui_internal.h"
#include "msdf-atlas-gen/types.h"
#include "rurouni/core/asset_manager.hpp"
#include "rurouni/core/components.hpp"
#include "rurouni/core/components/identifier.hpp"
#include "rurouni/core/components/texture.hpp"
#include "rurouni/core/components/transform.hpp"
#include "rurouni/editor.hpp"
#include "rurouni/editor/logger.hpp"
#include "rurouni/event/application_event.hpp"
#include "rurouni/event/event_system.hpp"
#include "rurouni/graphics/font.hpp"
#include "rurouni/graphics/spritesheet.hpp"
#include "rurouni/graphics/texture.hpp"
#include "rurouni/graphics/window.hpp"
#include "rurouni/math.hpp"
#include "rurouni/system/command.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/types/uuid.hpp"

#include <fa_solid_900.h>
#include <font_awesome_5.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <magic_enum.hpp>

#include <cfloat>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>

namespace rr::editor::ui {

std::unique_ptr<RRUIContext> g_rrContext;

RRUIContext& get_context() {
    return *g_rrContext;
}

void init(graphics::Window& window,
          const system::Path& sharedDataDir,
          const system::Path& userDataDir,
          const system::Path& userConfigDir) {
    // create context;
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    g_rrContext = std::make_unique<RRUIContext>();

    // configure io flags
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // imgui styling
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // create user config dir, if it doesn't exist
    if (!system::exists(userConfigDir)) {
        system::create_directories(userConfigDir);
    }

    // load ini file from shared or user dir
    io.IniFilename = NULL;
    if (system::exists(userConfigDir / "imgui.ini")) {
        ImGui::LoadIniSettingsFromDisk(
            (userConfigDir / "imgui.ini").string().c_str());
    } else {
        ImGui::LoadIniSettingsFromDisk(
            (sharedDataDir / "imgui.ini").string().c_str());
    }

    glm::vec2 contentScale = window.get_window_data().ContentScale;

    // init default font
    ImFont* caskaydia = io.Fonts->AddFontFromFileTTF(
        (sharedDataDir / "fonts/caskaydia_code_mono_regular.ttf").c_str(),
        g_rrContext->Style.FontSize * contentScale.x);
    g_rrContext->Fonts.Monospace_Regular = caskaydia;

    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    config.GlyphMinAdvanceX =
        15.0f * contentScale.x;  // Use if you want to make the icon monospaced
    static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFont* awesome = io.Fonts->AddFontFromMemoryTTF(
        (void*)fa_solid_900, sizeof(fa_solid_900),
        g_rrContext->Style.FontSize * contentScale.x, &config, icon_ranges);
    g_rrContext->Fonts.Icon = awesome;

    g_rrContext->Style.ToastContentFont = caskaydia;
    g_rrContext->Style.ToastIconFont = awesome;
    g_rrContext->Style.ToastBorderRounding = 5.0f;

    RRUIToastTypeStyle& infoConfig =
        g_rrContext->Style.ToastTypeStyle[ToastType::Info];
    infoConfig.BackgroundColor =
        ImVec4(0.0f / 255.0f, 58.0f / 255.0f, 96.0f / 255.0f, 1.0f);
    infoConfig.IconCodepoint = ICON_FA_CHECK_CIRCLE;

    RRUIToastTypeStyle& warnConfig =
        g_rrContext->Style.ToastTypeStyle[ToastType::Warning];
    warnConfig.BackgroundColor =
        ImVec4(86.0f / 255.0f, 0.0f / 255.0f, 96.0f / 255.0f, 1.0f);
    warnConfig.IconCodepoint = ICON_FA_EXCLAMATION_TRIANGLE;

    RRUIToastTypeStyle& errorConfig =
        g_rrContext->Style.ToastTypeStyle[ToastType::Error];
    errorConfig.BackgroundColor =
        ImVec4(96.0f / 255.0f, 0.0f / 255.0f, 58.0f / 255.0f, 1.0f);
    errorConfig.IconCodepoint = ICON_FA_TIMES_CIRCLE;
    errorConfig.DisplayTime = std::numeric_limits<float>::max();

    RRUIToastTypeStyle& successConfig =
        g_rrContext->Style.ToastTypeStyle[ToastType::Success];
    successConfig.BackgroundColor =
        ImVec4(58.0f / 255.0f, 96.0f / 255.0f, 0.0f / 255.0f, 1.0f);
    successConfig.IconCodepoint = ICON_FA_CHECK_CIRCLE;

    // init icons
    // TODO FileOpen distinctive icon
    g_rrContext->Icons.FileOpen =
        std::make_unique<graphics::Texture>(graphics::ImageTextureSpecification{
            "icon-file-open", UUID::create(),
            sharedDataDir / "textures/icons/file-import.png",
            graphics::TextureDistanceFieldType::None});
    g_rrContext->Icons.File =
        std::make_unique<graphics::Texture>(graphics::ImageTextureSpecification{
            "icon-file", UUID::create(),
            sharedDataDir / "textures/icons/file.png",
            graphics::TextureDistanceFieldType::None});
    g_rrContext->Icons.FileImport =
        std::make_unique<graphics::Texture>(graphics::ImageTextureSpecification{
            "icon-file-import", UUID::create(),
            sharedDataDir / "textures/icons/file-import.png",
            graphics::TextureDistanceFieldType::None});
    g_rrContext->Icons.Folder =
        std::make_unique<graphics::Texture>(graphics::ImageTextureSpecification{
            "icon-folder", UUID::create(),
            sharedDataDir / "textures/icons/folder.png",
            graphics::TextureDistanceFieldType::None});
    g_rrContext->Icons.Save =
        std::make_unique<graphics::Texture>(graphics::ImageTextureSpecification{
            "icon-floppy-pen", UUID::create(),
            sharedDataDir / "textures/icons/floppy-pen.png",
            graphics::TextureDistanceFieldType::None});
    g_rrContext->Icons.SquarePlus =
        std::make_unique<graphics::Texture>(graphics::ImageTextureSpecification{
            "icon-square-plus", UUID::create(),
            sharedDataDir / "textures/icons/square-plus.png",
            graphics::TextureDistanceFieldType::None});

    // graphics context initialization
    const char* glsl_version = "#version 410";
    ImGui_ImplGlfw_InitForOpenGL(window.get_native_window(), true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void terminate() {
    g_rrContext.reset();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void set_display_size(const glm::ivec2 size) {
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.DisplaySize = ImVec2(size.x, size.y);
}

void update(float dt, const system::Path& userConfigDir) {
    // set deltatime
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.DeltaTime = dt;

    // save new imgui ini file
    if (io.WantSaveIniSettings) {
        ImGui::SaveIniSettingsToDisk((userConfigDir / "imgui.ini").c_str());
    }
}

void begin() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void end() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void show_dockspace() {
    static bool opt_open = true;
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent
    // window not dockable into, because it would be confusing to have two
    // docking targets within each others.
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar |
                        ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus |
                        ImGuiWindowFlags_NoNavFocus;
    } else {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will
    // render our background and handle the pass-thru hole, so we ask Begin() to
    // not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window
    // is collapsed). This is because we want to keep our DockSpace() active. If
    // a DockSpace() is inactive, all active windows docked into it will lose
    // their parent and become undocked. We cannot preserve the docking
    // relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in
    // limbo and never being visible.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Rurouni Dockspace", &opt_open, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Submit the DockSpace
    // dockspace is always enabled
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("File Browser", NULL,
                            &g_rrContext->ShowFileBrowser);
            ImGui::MenuItem("Scene Manager", NULL,
                            &g_rrContext->ShowSceneManager);
            ImGui::MenuItem("Properties", NULL, &g_rrContext->ShowProperties);
            ImGui::MenuItem("ImGui Demo", NULL, &g_rrContext->ShowImGuiDemo);

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::End();
}

void show_imgui_demo() {
    if (g_rrContext->ShowImGuiDemo)
        ImGui::ShowDemoWindow(&g_rrContext->ShowImGuiDemo);
}

void show_startup_splash(
    std::unordered_map<UUID, ModuleHistoryItem>& history,
    std::function<bool(const system::Path&)> importModuleFunc,
    std::function<bool(const UUID&)> openModuleFunc,
    std::function<bool(const system::Path&, const std::string&)>
        createModuleFunc,
    std::function<void()> exitFunc) {
    // variables
    static UUID selectedId = UUID();
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();

    // Always center this window always
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    if (ImGui::Begin(
            "Start Splash", &g_rrContext->ShowStartupSplash,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)) {
        /////////////////////////////
        //>>>>>  HISTORY
        // TODO validation: does the project still exist?
        if (ImGui::BeginListBox("##history")) {
            for (auto& [k, v] : history) {
                ImGui::PushID(k.to_string().c_str());
                const bool is_selected = (selectedId == k);
                if (ImGui::Selectable(v.Name.c_str(), is_selected))
                    selectedId = selectedId == k ? UUID() : k;

                // Set the initial focus when opening the combo (scrolling +
                // keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();

                ImGui::PopID();
            }
            ImGui::EndListBox();
        }
        //<<<<<  HISTORY
        /////////////////////////////

        /////////////////////////////
        //>>>>>  CREATE NEW MODULE
        if (ImGui::Button("New", ImVec2(120, 0))) {
            ImGui::OpenPopup("Create##Module");
        }
        show_create_module_modal(createModuleFunc);

        ImGui::SameLine();
        /////////////////////////////
        //>>>>>  IMPORT MODULE
        if (ImGui::Button("Import", ImVec2(120, 0))) {
            ImGui::OpenPopup("Import##Module");
        }
        show_import_module_modal(importModuleFunc);

        ImGui::SameLine();
        /////////////////////////////
        //>>>>>  OPEN MODULE
        ImGui::BeginDisabled(selectedId.is_null());
        if (ImGui::Button("Open", ImVec2(120, 0))) {
            bool success = openModuleFunc(selectedId);

            if (!success) {
                push_toast(Toast(ToastType::Error, "open module",
                                 "failed opening module.\nid: {}\nsee error "
                                 "log for more information.",
                                 selectedId));
            }
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        /////////////////////////////
        //>>>>>  EXIT
        if (ImGui::Button("Exit", ImVec2(120, 0))) {
            exitFunc();
        }

        ImGui::End();
    }
}

void show_create_module_modal(
    std::function<bool(const system::Path&, const std::string&)>
        createModuleFunc) {
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Create##Module", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        static char name[32] = "";
        static char path[256] = "";

        if (ImGui::BeginTable("table", 3, ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Name");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText("##name", name, 32);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Path");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText("##path", path, 256);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(0.5, 0.5, 0.5, 0.8));
            ImGui::TableNextColumn();
            ImGui::PushFont(g_rrContext->Fonts.Icon);
            if (ImGui::Button(ICON_FA_FILE_IMPORT)) {
                std::string result = system::execute_command(
                    "zenity --file-selection --directory");
                result.copy(path, result.size());
            }
            ImGui::PopFont();
            ImGui::PopStyleColor(2);
            ImGui::SetItemTooltip("Open file browser.");

            ImGui::EndTable();
        }

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            bool success = createModuleFunc(path, name);
            if (success) {
                memset(name, 0, 32);
                memset(path, 0, 256);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            memset(name, 0, 32);
            memset(path, 0, 256);
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void show_import_module_modal(
    std::function<bool(const system::Path&)> importModuleFunc) {
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Import##Module", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        static char name[32] = "";
        static char path[256] = "";

        if (ImGui::BeginTable("table", 3, ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Name");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText("##name", name, 32);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Path");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText("##path", path, 256);
            ImGui::TableNextColumn();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(0.5, 0.5, 0.5, 0.8));
            ImGui::PushFont(g_rrContext->Fonts.Icon);
            if (ImGui::Button(ICON_FA_FILE_IMPORT)) {
                std::string result = system::execute_command(
                    "zenity --file-selection --directory");
                result.copy(path, result.size());
            }
            ImGui::PopFont();
            ImGui::PopStyleColor(2);
            ImGui::SetItemTooltip("Open file browser.");

            ImGui::EndTable();
        }

        ///

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            bool success = importModuleFunc(path);
            if (success) {
                memset(name, 0, 32);
                memset(path, 0, 256);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            memset(name, 0, 32);
            memset(path, 0, 256);
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void show_asset_manager(
    core::AssetManager& assetManager,
    std::function<bool(graphics::ImageTextureSpecification&)>
        importImageTextureFunc,
    std::function<bool(graphics::SpritesheetSpecification&)>
        importSpritesheetFunc,
    std::function<bool(std::unordered_map<int, graphics::SpriteSpecification>&)>
        importSpritesFunc,
    std::function<bool(graphics::ShaderSpecification&)> importShaderFunc,
    std::function<bool(ui::FontImportSpecification&,
                       ui::FontImportConfiguration&)> importFontFunc) {
    if (!g_rrContext->ShowAssetManager)
        return;

    if (ImGui::Begin("Assets", &g_rrContext->ShowAssetManager)) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.5, 0.5, 0.5, 0.8));
        size_t image = g_rrContext->Icons.FileImport->get_renderer_id();
        if (ImGui::ImageButton((ImTextureID)image, g_rrContext->Style.IconSize,
                               {0, 0}, {1, 1})) {
            ImGui::OpenPopup("Import##Assets");
        }
        ImGui::SetItemTooltip("Open import asset modal");
        ImGui::PopStyleColor(2);
        show_import_assets_modal(assetManager, importImageTextureFunc,
                                 importSpritesheetFunc, importSpritesFunc,
                                 importShaderFunc, importFontFunc);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.5, 0.5, 0.5, 0.8));
        image = g_rrContext->Icons.Save->get_renderer_id();
        if (ImGui::ImageButton((ImTextureID)image, g_rrContext->Style.IconSize,
                               {0, 0}, {1, 1})) {
            assetManager.write_asset_configuration();
        }
        ImGui::SetItemTooltip("Save asset specifications");
        ImGui::PopStyleColor(2);

        ImGui::Separator();

        ImGui::BeginChild("contents");

        if (ImGui::CollapsingHeader("Texture")) {
            for (auto& [textureId, textureSpec] :
                 assetManager.get_image_texture_database()) {
                ImGui::PushID(
                    textureId.to_string().append("##textures").c_str());
                if (ImGui::Selectable(textureSpec.Name.c_str())) {
                    ui::push_property_tab(PropertyTab(PropertyTabType::Texture,
                                                      {textureSpec.Id},
                                                      textureSpec.Id));
                }

                if (ImGui::BeginDragDropSource()) {
                    ImGui::SetDragDropPayload("TEXTURE_SPECIFICATION",
                                              &textureSpec,
                                              sizeof(textureSpec));

                    ImGui::EndDragDropSource();
                }
                ImGui::PopID();
            }
        }

        // TODO DataTextures

        if (ImGui::CollapsingHeader("Sprites")) {
            std::unordered_map<
                UUID, std::unordered_map<UUID, graphics::SpriteSpecification*>>
                subTextureMap;

            for (auto& [spriteId, spriteSpec] :
                 assetManager.get_sprite_database()) {
                subTextureMap[spriteSpec.SpritesheetId][spriteId] = &spriteSpec;
            }

            for (auto& [sheetId, spriteEntry] : subTextureMap) {
                ImGui::PushID(sheetId.to_string().append("##sprites").c_str());
                auto& sheetSpec =
                    assetManager.get_spritesheet_database()[sheetId];

                auto flags = ImGuiTreeNodeFlags_SpanFullWidth;
                if (ImGui::TreeNodeEx(sheetSpec.Name.c_str(), flags)) {
                    for (auto& [spriteId, spriteSpecPtr] : spriteEntry) {
                        ImGui::PushID(
                            spriteId.to_string().append("##sprites").c_str());
                        if (ImGui::Selectable(spriteSpecPtr->Name.c_str())) {
                            ui::push_property_tab(PropertyTab(
                                PropertyTabType::Sprite, {spriteSpecPtr->Id},
                                spriteSpecPtr->Id));
                        }

                        if (ImGui::BeginDragDropSource()) {
                            ImGui::SetDragDropPayload("SPRITE_SPECIFICATION",
                                                      spriteSpecPtr,
                                                      sizeof(*spriteSpecPtr));

                            ImGui::EndDragDropSource();
                        }
                        ImGui::PopID();
                    }

                    ImGui::TreePop();
                }

                ImGui::PopID();
            }
        }

        if (ImGui::CollapsingHeader("Shaders")) {
            for (auto& [k, v] : assetManager.get_shader_database()) {
                ImGui::Text("%s", v.Name.c_str());
            }
        }

        if (ImGui::CollapsingHeader("Fonts")) {
            for (auto& [k, v] : assetManager.get_font_database()) {
                ImGui::PushID(k.to_string().append("##fonts").c_str());
                if (ImGui::Selectable(v.Name.c_str())) {
                    ui::push_property_tab(
                        PropertyTab(PropertyTabType::Font, {v.Id}, v.Id));
                }

                ImGui::PopID();
            }
        }

        ImGui::EndChild();
    }

    ImGui::End();
}

void show_import_assets_modal(
    core::AssetManager& assetManager,
    std::function<bool(graphics::ImageTextureSpecification&)>
        importImageTextureFunc,
    std::function<bool(graphics::SpritesheetSpecification&)>
        importSpritesheetFunc,
    std::function<bool(std::unordered_map<int, graphics::SpriteSpecification>&)>
        importSpritesFunc,
    std::function<bool(graphics::ShaderSpecification&)> importShaderFunc,
    std::function<bool(ui::FontImportSpecification&,
                       ui::FontImportConfiguration&)> importFontFunc) {
    static size_t selectedTabTypeHash = typeid(graphics::Texture).hash_code();

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;

    if (ImGui::BeginPopupModal("Import##Assets", NULL, flags)) {
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("Asset Type", tab_bar_flags)) {
            // TODO how to handle data textures?
            if (ImGui::BeginTabItem("Texture")) {
                selectedTabTypeHash = typeid(graphics::Texture).hash_code();
                show_import_assets_tab_texture(assetManager,
                                               importImageTextureFunc);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Spritesheet")) {
                selectedTabTypeHash = typeid(graphics::Texture).hash_code();
                show_import_assets_tab_spritesheet(assetManager,
                                                   importSpritesheetFunc);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Sprite")) {
                selectedTabTypeHash = typeid(graphics::Sprite).hash_code();
                show_import_assets_tab_sprite(assetManager, importSpritesFunc);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Shader")) {
                selectedTabTypeHash = typeid(graphics::Shader).hash_code();
                show_import_assets_tab_shader(assetManager, importShaderFunc);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Font")) {
                selectedTabTypeHash = typeid(graphics::Font).hash_code();
                show_import_assets_tab_font(assetManager, importFontFunc);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::EndPopup();
    }
}

void show_import_assets_tab_texture(
    core::AssetManager& assetManager,
    std::function<bool(graphics::ImageTextureSpecification&)>
        importImageTextureFunc) {
    static std::unique_ptr<graphics::Texture> texture;
    static graphics::ImageTextureSpecification spec;
    static char path[256] = "";

    if (ImGui::BeginTable("Table##TextureImport", 3,
                          ImGuiTableFlags_SizingStretchProp)) {
        char name[32] = "";
        memset(name, 0, sizeof(name));
        strcpy(name, spec.Name.c_str());
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Name");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::InputText("##Name", name, 32)) {
            spec.Name = std::string(name);
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Path");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::InputText("##Path", path, 256)) {
            if (system::exists(path) && system::is_regular_file(path) &&
                system::Path(path).extension() == ".png") {
                texture = std::make_unique<graphics::Texture>(spec);
            } else {
                texture.reset();
            }
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.5, 0.5, 0.5, 0.8));
        ImGui::PushFont(g_rrContext->Fonts.Icon);
        ImGui::TableNextColumn();
        if (ImGui::Button(ICON_FA_FILE_IMPORT)) {
            std::string result =
                system::execute_command("zenity --file-selection");
            result.copy(path, result.size());

            if (system::exists(path) && system::is_regular_file(path) &&
                system::Path(path).extension() == ".png") {
                spec.Filepath = system::Path(path);
                texture = std::make_unique<graphics::Texture>(spec);
            } else {
                texture.reset();
            }
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(2);
        ImGui::SetItemTooltip("Open file browser.");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Preview");
        if (texture != nullptr) {
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(240.0f);

            size_t textureId = texture->get_renderer_id();
            auto textureSize = texture->get_size();
            ImVec2 uv_min = ImVec2(0.0f, 0.0f);  // Top-left
            ImVec2 uv_max = ImVec2(1.0f, 1.0f);  // Lower-right
            float ar = (float)textureSize.x / (float)textureSize.y;
            textureSize.x = 240.0f;
            textureSize.y = 240.0f * ar;
            ImGui::Image((ImTextureID)textureId,
                         ImVec2(textureSize.x, textureSize.y), uv_min, uv_max);
        }

        ImGui::EndTable();
    }

    ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 3.0f);

    if (ImGui::Button("OK", ImVec2(120, 0))) {
        bool success = importImageTextureFunc(spec);

        if (success) {
            spec = graphics::ImageTextureSpecification();
            texture.reset();
            ImGui::CloseCurrentPopup();
        } else {
            push_toast(Toast(
                ToastType::Error, "import texture",
                "failed importing texture. see log for more information."));
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        spec = graphics::ImageTextureSpecification();
        texture.reset();
        ImGui::CloseCurrentPopup();
    }
}

void show_import_assets_tab_spritesheet(
    core::AssetManager& assetManager,
    std::function<bool(graphics::SpritesheetSpecification&)>
        importSpritesheetFunc) {
    static graphics::SpritesheetSpecification spec;
    static std::string previewStr = "select texture";

    if (ImGui::BeginTable("Table##TextureImport", 3,
                          ImGuiTableFlags_SizingStretchProp)) {
        char name[32] = "";
        memset(name, 0, sizeof(name));
        strcpy(name, spec.Name.c_str());
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Name");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::InputText("##Name", name, 32)) {
            spec.Name = std::string(name);
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Cell Count");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        // TODO doesn't clamp
        if (ImGui::DragInt2("##CellCount", math::value_ptr(spec.CellCount), 1,
                            0)) {
        }

        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Texture");
            ImGui::TableNextColumn();
            auto& texSpecs = assetManager.get_image_texture_database();
            if (ImGui::BeginCombo("##Type", previewStr.c_str())) {
                for (auto& [id, texSpec] : texSpecs) {
                    const bool is_selected = (id == spec.TextureId);
                    if (ImGui::Selectable(texSpec.Name.c_str(), is_selected)) {
                        spec.TextureId = id;
                        previewStr = texSpec.Name;
                    }

                    // Set the initial focus when opening the combo (scrolling +
                    // keyboard navigation focus)
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Preview");
        if (assetManager.has_image_texture_specification(spec.TextureId)) {
            auto texturePtr = assetManager.get_texture(spec.TextureId).lock();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(240.0f);

            size_t textureId = texturePtr->get_renderer_id();
            auto textureSize = texturePtr->get_size();
            ImVec2 uv_min = ImVec2(0.0f, 0.0f);  // Top-left
            ImVec2 uv_max = ImVec2(1.0f, 1.0f);  // Lower-right
            float ar = (float)textureSize.x / (float)textureSize.y;
            textureSize.x = 240.0f;
            textureSize.y = 240.0f * ar;

            ImVec2 imagePosition = ImGui::GetCursorScreenPos();
            ImGui::SetNextItemAllowOverlap();
            ImGui::Image((ImTextureID)textureId,
                         ImVec2(textureSize.x, textureSize.y), uv_min, uv_max);

            /////// grid
            float spriteSizeX = (float)textureSize.x / spec.CellCount.x;
            float spriteSizeY = (float)textureSize.y / spec.CellCount.y;

            float positionX = imagePosition.x;
            float positionY = imagePosition.y;
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
            ImGui::PushStyleColor(ImGuiCol_Border,
                                  ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  (ImVec4)ImColor(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  (ImVec4)ImColor(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                  (ImVec4)ImColor(0.0f, 0.0f, 0.0f, 0.0f));
            for (int y = 0; y < spec.CellCount.y; y++) {
                for (int x = 0; x < spec.CellCount.x; x++) {
                    int index = (y * spec.CellCount.x) + x;

                    ImGui::SetCursorScreenPos(ImVec2(positionX, positionY));
                    ImGui::PushID(fmt::format("{}{}", x, y).c_str());
                    if (ImGui::Button("##button",
                                      ImVec2(spriteSizeX, spriteSizeY))) {
                    }
                    ImGui::PopID();

                    positionX += spriteSizeX;
                }
                positionX = imagePosition.x;
                positionY += spriteSizeY;
            }
            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar(1);
        }

        ImGui::EndTable();
    }

    ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 3.0f);

    if (ImGui::Button("OK", ImVec2(120, 0))) {
        bool success = importSpritesheetFunc(spec);

        if (success) {
            spec = graphics::SpritesheetSpecification();
            previewStr = "select texture";
            ImGui::CloseCurrentPopup();
        } else {
            push_toast(Toast(
                ToastType::Error, "import texture",
                "failed importing texture. see log for more information."));
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        spec = graphics::SpritesheetSpecification();
        previewStr = "select texture";
        ImGui::CloseCurrentPopup();
    }
}

void show_import_assets_tab_sprite(
    core::AssetManager& assetManager,
    std::function<bool(std::unordered_map<int, graphics::SpriteSpecification>&)>
        importSpritesFunc) {
    static std::unordered_map<int, graphics::SpriteSpecification>
        markedSpritesMap;
    static std::unordered_map<int, graphics::SpriteSpecification>
        existingSpritesMap;
    static std::optional<graphics::SpritesheetSpecification> selectedSheetSpec;
    static std::optional<graphics::SpriteSpecification> selectedSpriteSpec;

    ImGui::BeginGroup();

    // texture selection
    std::string comboHint = selectedSheetSpec.has_value()
                                ? selectedSheetSpec->Name
                                : "Select Texture";
    if (ImGui::BeginCombo("Spritesheet", comboHint.c_str())) {
        for (const auto& [id, sheetSpec] :
             assetManager.get_spritesheet_database()) {
            const bool is_selected = (selectedSheetSpec.has_value()) &&
                                     (selectedSheetSpec->Id == id);
            if (ImGui::Selectable(sheetSpec.Name.c_str(), is_selected)) {
                selectedSheetSpec = sheetSpec;

                auto& spriteRegistry = assetManager.get_sprite_database();

                // copy existing specs into local specMap
                for (auto& [k, v] : spriteRegistry) {
                    // TODO make != operator for UUID
                    if (!(v.SpritesheetId == selectedSheetSpec->Id))
                        continue;

                    // calculate single digit index inside texture
                    int index =
                        (v.CellIndex.y * selectedSheetSpec->CellCount.x) +
                        v.CellIndex.x;
                    existingSpritesMap[index] = v;
                }
            }

            // Set the initial focus when opening the combo (scrolling +
            // keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    // sprite selection
    bool deleteAction = false;
    if (ImGui::BeginListBox("Marked Sprites")) {
        for (auto it = markedSpritesMap.begin();
             it != markedSpritesMap.end();) {
            ImGui::PushID(it->second.Id.to_string().c_str());

            const bool is_selected = (selectedSpriteSpec.has_value()) &&
                                     (selectedSpriteSpec->Id == it->second.Id);

            if (ImGui::Button("X")) {
                if (is_selected)
                    selectedSpriteSpec = {};

                it = markedSpritesMap.erase(it);

                ImGui::PopID();
                continue;
            }

            ImGui::SameLine();

            if (ImGui::Selectable(it->second.Name.c_str(), is_selected)) {
                selectedSpriteSpec = it->second;
            }

            // Set the initial focus when opening the combo (scrolling +
            // keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();

            ImGui::PopID();
            it++;
        }
        ImGui::EndListBox();
    }

    // draw sprite info if one is selected
    if (selectedSpriteSpec.has_value()) {
        ImGui::Text("%s", selectedSpriteSpec->Name.c_str());
    }

    ImGui::EndGroup();

    // draw nothing else, if there are no sprites in texture
    if (!selectedSheetSpec.has_value())
        return;

    ImGui::SameLine();

    int spriteCountX = selectedSheetSpec->CellCount.x;
    int spriteCountY = selectedSheetSpec->CellCount.y;
    float spriteSizeX_uv = 1.0f / spriteCountX;
    float spriteSizeY_uv = 1.0f / spriteCountY;

    // draw image
    auto texture =
        assetManager.get_texture(selectedSheetSpec->TextureId).lock();
    // TODO has_value() check?
    size_t textureId = texture->get_renderer_id();
    auto textureSize = texture->get_size();
    ImVec2 uv_min = ImVec2(0.0f, 0.0f);  // Top-left
    ImVec2 uv_max = ImVec2(1.0f, 1.0f);  // Lower-right
    ImVec2 region = ImGui::GetContentRegionAvail();
    float ar = (float)textureSize.x / (float)textureSize.y;

    // textureSize.x = region.x;
    // textureSize.y = region.x * ar;
    // TODO not-hardcoded values
    textureSize.x = 512;
    textureSize.y = 512;

    ImVec2 imagePosition = ImGui::GetCursorScreenPos();
    ImGui::SetNextItemAllowOverlap();
    ImGui::Image((void*)textureId, ImVec2(textureSize.x, textureSize.y), uv_min,
                 uv_max);

    // TODO not-hardcoded values
    float spriteSizeX = 512.0f / selectedSheetSpec->CellCount.x;
    float spriteSizeY = 512.0f / selectedSheetSpec->CellCount.y;

    float positionX = imagePosition.x;
    float positionY = imagePosition.y;

    // draw sprite grid
    for (int y = 0; y < spriteCountY; y++) {
        for (int x = 0; x < spriteCountX; x++) {
            int index = (y * spriteCountX) + x;
            bool isInExistingMap =
                existingSpritesMap.find(index) != existingSpritesMap.end();
            bool isInMarkedMap =
                markedSpritesMap.find(index) != markedSpritesMap.end();

            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
            if (isInExistingMap) {
                ImGui::PushStyleColor(ImGuiCol_Button,
                                      (ImVec4)ImColor(0.1f, 0.2f, 0.9f, 0.5f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                      (ImVec4)ImColor(0.2f, 0.1f, 0.8f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                      (ImVec4)ImColor(0.0f, 0.8f, 0.8f, 0.5f));
            } else if (isInMarkedMap) {
                ImGui::PushStyleColor(ImGuiCol_Button,
                                      (ImVec4)ImColor(0.1f, 0.85f, 0.1f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                      (ImVec4)ImColor(0.1f, 0.85f, 0.1f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                      (ImVec4)ImColor(0.0f, 0.8f, 0.8f, 0.5f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button,
                                      (ImVec4)ImColor(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                      (ImVec4)ImColor(0.2f, 0.5f, 0.2f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                      (ImVec4)ImColor(0.0f, 0.8f, 0.8f, 0.5f));
            }

            ImGui::SetCursorScreenPos(ImVec2(positionX, positionY));
            ImGui::PushID(fmt::format("{}{}", x, y).c_str());
            if (ImGui::Button("##button", ImVec2(spriteSizeX, spriteSizeY))) {
                if (!isInExistingMap && !isInMarkedMap) {  // add sprite to map
                    // flipped y index
                    graphics::SpriteSpecification spec;
                    spec.Name = fmt::format("x[{}]y[{}]", x, y);
                    spec.Id = UUID::create();
                    spec.CellIndex = math::ivec2(x, y);
                    spec.SpritesheetId = selectedSheetSpec->Id;
                    spec.CellSpread = math::ivec2(1.0f);

                    markedSpritesMap[index] = spec;
                    selectedSpriteSpec = spec;
                } else if (isInExistingMap) {
                    selectedSpriteSpec = existingSpritesMap[index];
                } else if (isInMarkedMap) {
                    selectedSpriteSpec = markedSpritesMap[index];
                }
            }
            ImGui::PopID();

            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar();

            positionX += spriteSizeX;
        }
        positionX = imagePosition.x;
        positionY += spriteSizeY;
    }

    ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 3.0f);

    if (ImGui::Button("OK", ImVec2(120, 0))) {
        bool success = importSpritesFunc(markedSpritesMap);

        if (success) {
            selectedSheetSpec = std::nullopt;
            selectedSpriteSpec = std::nullopt;
            markedSpritesMap.clear();
            existingSpritesMap.clear();
            ImGui::CloseCurrentPopup();
        } else {
            push_toast(Toast(
                ToastType::Error, "import sprites",
                "failed importing sprites. see log for more information."));
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        selectedSheetSpec = std::nullopt;
        selectedSpriteSpec = std::nullopt;
        markedSpritesMap.clear();
        existingSpritesMap.clear();
        ImGui::CloseCurrentPopup();
    }
}

void show_import_assets_tab_font(
    core::AssetManager& assetManager,
    std::function<bool(ui::FontImportSpecification&,
                       ui::FontImportConfiguration&)> importFontFunc) {
    static FontImportSpecification fontSpec;
    static FontImportConfiguration fontConfig;

    if (ImGui::BeginTable("Table##FontImport", 3,
                          ImGuiTableFlags_SizingStretchProp)) {
        char name[32] = "";
        memset(name, 0, sizeof(name));
        strcpy(name, fontSpec.FontName.c_str());
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Name");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::InputText("##Name", name, 32)) {
            fontSpec.FontName = std::string(name);
        }

        {
            char path[256] = "";
            memset(path, 0, sizeof(path));
            strcpy(path, fontSpec.FontFilepath.c_str());
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Input Path");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::InputText("##InputPath", path, 256)) {
                fontSpec.FontFilepath = std::string(path);
            }

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(0.5, 0.5, 0.5, 0.8));
            ImGui::PushFont(g_rrContext->Fonts.Icon);
            ImGui::TableNextColumn();
            if (ImGui::Button(ICON_FA_FILE_IMPORT)) {
                std::string result =
                    system::execute_command("zenity --file-selection");
                result.copy(path, result.size());
                fontSpec.FontFilepath = result;
            }
            ImGui::PopFont();
            ImGui::PopStyleColor(2);
            ImGui::SetItemTooltip("Open file browser.");
        }

        {
            char path[256] = "";
            memset(path, 0, sizeof(path));
            strcpy(path, fontConfig.ImageFilepath.c_str());
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Output Path");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::InputText("##OutputPath", path, 256)) {
                fontConfig.ImageFilepath = std::string(path);
            }

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(0.5, 0.5, 0.5, 0.8));
            ImGui::PushFont(g_rrContext->Fonts.Icon);
            ImGui::TableNextColumn();
            if (ImGui::Button(ICON_FA_FILE_IMPORT)) {
                std::string result =
                    system::execute_command("zenity --file-selection");
                result.copy(path, result.size());
                fontSpec.FontFilepath = result;
            }
            ImGui::PopFont();
            ImGui::PopStyleColor(2);
            ImGui::SetItemTooltip("Open file browser.");
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Size");
        ImGui::TableNextColumn();
        // ImGui::SetNextItemWidth(120.0f);
        float size = fontConfig.EmSize;
        if (ImGui::DragFloat("##Size", &size, 1.0f, 0.0f,
                             std::numeric_limits<float>::max())) {
            fontConfig.EmSize = size;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Font Scale");
        ImGui::TableNextColumn();
        // ImGui::SetNextItemWidth(120.0f);
        float scale = fontSpec.FontScale;
        if (ImGui::DragFloat("##FontScale", &scale, 1.0f, 0.0f,
                             std::numeric_limits<float>::max())) {
            fontSpec.FontScale = scale;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Miter Limit");
        ImGui::TableNextColumn();
        // ImGui::SetNextItemWidth(120.0f);
        float miter = fontConfig.MiterLimit;
        if (ImGui::DragFloat("##MiterLimit", &miter, 1.0f, 0.0f,
                             std::numeric_limits<float>::max())) {
            fontConfig.MiterLimit = miter;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Px Range");
        ImGui::TableNextColumn();
        // ImGui::SetNextItemWidth(120.0f);
        float range = fontConfig.PxRange;
        if (ImGui::DragFloat("##PxRange", &range, 1.0f, 0.0f,
                             std::numeric_limits<float>::max())) {
            fontConfig.PxRange = range;
        }

        {
            constexpr auto type_entries =
                magic_enum::enum_entries<msdf_atlas::ImageType>();
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Type");
            ImGui::TableNextColumn();
            if (ImGui::BeginCombo(
                    "##Type",
                    type_entries[static_cast<int>(fontConfig.ImageType)]
                        .second.data())) {
                for (auto& [type, strView] : type_entries) {
                    const bool is_selected = (type == fontConfig.ImageType);
                    if (ImGui::Selectable(strView.data(), is_selected))
                        fontConfig.ImageType = type;

                    // Set the initial focus when opening the combo (scrolling +
                    // keyboard navigation focus)
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }

        {
            constexpr auto format_entries =
                magic_enum::enum_entries<msdf_atlas::ImageFormat>();
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Type");
            ImGui::TableNextColumn();
            if (ImGui::BeginCombo(
                    "##Format",
                    format_entries[static_cast<int>(fontConfig.ImageFormat)]
                        .second.data())) {
                for (auto& [format, strView] : format_entries) {
                    const bool is_selected = (format == fontConfig.ImageFormat);
                    if (ImGui::Selectable(strView.data(), is_selected))
                        fontConfig.ImageFormat = format;

                    // Set the initial focus when opening the combo (scrolling +
                    // keyboard navigation focus)
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }

        ImGui::EndTable();
    }

    ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 3.0f);

    if (ImGui::Button("OK", ImVec2(120, 0))) {
        bool success = importFontFunc(fontSpec, fontConfig);

        if (success) {
            fontSpec = FontImportSpecification();
            fontConfig = FontImportConfiguration();
            ImGui::CloseCurrentPopup();
        } else {
            push_toast(
                Toast(ToastType::Error, "import font",
                      "failed importing font. see log for more information."));
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        fontSpec = FontImportSpecification();
        fontConfig = FontImportConfiguration();
        ImGui::CloseCurrentPopup();
    }
}

void show_import_assets_tab_shader(
    core::AssetManager& assetManager,
    std::function<bool(graphics::ShaderSpecification&)> importShaderFunc) {
    if (ImGui::Button("OK", ImVec2(120, 0))) {
        graphics::ShaderSpecification spec;
        bool success = importShaderFunc(spec);

        if (success) {
            ImGui::CloseCurrentPopup();
        } else {
            push_toast(Toast(
                ToastType::Error, "import shader",
                "failed importing shader. see log for more information."));
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        ImGui::CloseCurrentPopup();
    }
}

void show_scene_manager(core::Scene& scene,
                        std::function<void(const system::Path&)> changeSceneFn,
                        std::function<void(const system::Path&)> saveSceneFn) {
    if (!g_rrContext->ShowSceneManager)
        return;

    if (ImGui::Begin("Scene", &g_rrContext->ShowSceneManager)) {
        // dummy layer over whole content region to allow drag and drop
        // in whole window
        ImVec2 size = ImGui::GetContentRegionAvail();
        ImGui::SetNextItemAllowOverlap();
        ImGui::Dummy(size);
        ImVec2 pos = ImGui::GetItemRectMin();
        ImGui::SetCursorScreenPos(pos);

        // drag and drop scene file
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(
                    "CONTENT_BROWSER_SCENE_PATH")) {
                system::Path path = system::Path((const char*)payload->Data);

                changeSceneFn(path);
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.5, 0.5, 0.5, 0.8));

        size_t textureId =
            static_cast<size_t>(g_rrContext->Icons.Save->get_renderer_id());
        if (ImGui::ImageButton((ImTextureID)textureId,
                               g_rrContext->Style.IconSize, {0, 0}, {1, 1})) {
            saveSceneFn("");
        }

        ImGui::SetItemTooltip("Save scene");
        ImGui::PopStyleColor(2);

        ImGui::Separator();

        char buffer[32];
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, scene.get_name().c_str());
        if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
            scene.set_name(std::string(buffer));
        }

        // TODO camera
        // auto layerCamEnttId = scene.get_scene_state().LayerCameraId;
        // memset(buffer, 0, sizeof(buffer));
        // strcpy(buffer, std::to_string((uint32_t)layerCamEnttId).c_str());
        // if (ImGui::InputText("Layer Camera Id", buffer, sizeof(buffer))) {
        //     scene.set_camera_id(types::UUID(std::string(buffer)));
        // }

        if (ImGui::TreeNode("Layers")) {
            for (auto& layer : scene.get_layer_stack()) {
                ImGui::Text("%s", layer->get_name().c_str());
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Debug Layer")) {
            if (auto debugLayer = scene.get_debug_layer()) {
                ImGui::Text("%s", debugLayer->get_name().c_str());
            }

            ImGui::TreePop();
        }

        // get registry reference
        auto& registry = scene.get_registry();

        if (ImGui::TreeNode("Registry")) {
            // create view over all entities in registry
            // and draw each entity node
            auto view = registry.view<entt::entity>();
            for (auto&& [entityId] : view.each()) {
                // function variables
                auto& identifier =
                    registry.get<core::components::Identifier>(entityId);

                ImGui::PushID(identifier.Uuid.to_string().c_str());

                // selection when left clicking an entity
                bool selected = g_rrContext->SelectedEntity.has_value() &&
                                g_rrContext->SelectedEntity.value() == entityId;
                if (ImGui::Selectable(identifier.Name.c_str(), selected)) {
                    push_property_tab(PropertyTab(PropertyTabType::Entity,
                                                  {entityId}, identifier.Uuid));
                }

                // popup when right clicking on entity
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Delete Entity")) {
                        registry.destroy(entityId);

                        remove_property_tab(PropertyTabType::Entity,
                                            {entityId});
                    }

                    ImGui::EndPopup();
                }

                ImGui::PopID();
            }

            // popup menu when right clicking on empty space
            int popupFlags = ImGuiPopupFlags_NoOpenOverItems |
                             ImGuiPopupFlags_MouseButtonRight;
            if (ImGui::BeginPopupContextWindow(0, popupFlags)) {
                // create a new entity
                if (ImGui::MenuItem("Create Entity")) {
                    entt::entity entity = scene.create_entity("New Emptity");
                }

                ImGui::EndPopup();
            }

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void show_properties(entt::registry& registry,
                     core::AssetManager& assetManager) {
    if (!g_rrContext->ShowProperties)
        return;

    if (ImGui::Begin("Properties", &g_rrContext->ShowProperties)) {
        ImGuiTabBarFlags tabBarFlags = ImGuiTabBarFlags_Reorderable |
                                       ImGuiTabBarFlags_AutoSelectNewTabs |
                                       ImGuiTabBarFlags_FittingPolicyScroll |
                                       ImGuiTabBarFlags_TabListPopupButton;
        if (ImGui::BeginTabBar("Property Type", tabBarFlags)) {
            for (auto iter = g_rrContext->PropertyTabs.begin();
                 iter != g_rrContext->PropertyTabs.end();) {
                ImGui::PushID(iter->Id.to_string().c_str());

                ImGuiTabItemFlags flags = 0;

                if (g_rrContext->SelectedPropertyTabId.has_value() &&
                    iter->Id == g_rrContext->SelectedPropertyTabId.value()) {
                    flags = ImGuiTabItemFlags_SetSelected;
                    g_rrContext->SelectedPropertyTabId = std::nullopt;
                }

                bool open = true;
                switch (iter->Type) {
                    case PropertyTabType::Entity:
                        show_property_tab_entity(iter->Data.Entity, registry,
                                                 assetManager, open, flags);
                        break;
                    case PropertyTabType::Texture:
                        show_property_tab_texture(iter->Data.AssetId,
                                                  assetManager, open, flags);
                        break;
                    case PropertyTabType::Sprite:
                        show_property_tab_sprite(iter->Data.AssetId,
                                                 assetManager, open, flags);
                        break;
                    case PropertyTabType::Font:
                        show_property_tab_font(iter->Data.AssetId, assetManager,
                                               open, flags);
                        break;
                    case PropertyTabType::Shader:
                        show_property_tab_shader();
                        break;
                    default:
                        require(false, "unhandled property tab");
                        break;
                }

                if (!open) {
                    g_rrContext->PropertyTabs.erase(iter);
                } else {
                    iter++;
                }

                ImGui::PopID();
            }

            ImGui::EndTabBar();
        }
    }

    ImGui::End();
}

void push_property_tab(const PropertyTab& tab) {
    g_rrContext->SelectedPropertyTabId = tab.Id;

    for (auto& tabEntry : g_rrContext->PropertyTabs) {
        if (tabEntry.Id == tab.Id) {
            // tab already pushed
            return;
        }
    }

    g_rrContext->PropertyTabs.push_back(std::move(tab));
}

void remove_property_tab(PropertyTabType type,
                         const PropertyTab::data_t& data) {
    switch (type) {
        case PropertyTabType::Entity:
            for (auto iter = g_rrContext->PropertyTabs.begin();
                 iter != g_rrContext->PropertyTabs.end(); iter++) {
                if (iter->Data.Entity == data.Entity) {
                    g_rrContext->PropertyTabs.erase(iter);
                    return;
                }
            }
            break;
        case PropertyTabType::Texture:
        case PropertyTabType::Sprite:
        case PropertyTabType::Shader:
        case PropertyTabType::Font:
            for (auto iter = g_rrContext->PropertyTabs.begin();
                 iter != g_rrContext->PropertyTabs.end(); iter++) {
                if (iter->Data.AssetId == data.AssetId) {
                    g_rrContext->PropertyTabs.erase(iter);
                    return;
                }
            }
            break;
        default:
            require(false, "unhandled property tab type.");
            break;
    }
}

void show_property_tab_entity(entt::entity entity,
                              entt::registry& registry,
                              core::AssetManager& assetManager,
                              bool& open,
                              ImGuiSelectableFlags flags) {
    auto& identification = registry.get<core::components::Identifier>(entity);
    ImGui::PushID(identification.Uuid.to_string().c_str());

    if (ImGui::BeginTabItem(identification.Name.c_str(), &open, flags)) {
        ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, 5.0f);

        // header buttons
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.5, 0.5, 0.5, 0.8));

        size_t textureId = static_cast<size_t>(
            g_rrContext->Icons.SquarePlus->get_renderer_id());
        if (ImGui::ImageButton((ImTextureID)textureId,
                               g_rrContext->Style.IconSize, {0, 0}, {1, 1})) {
            ImGui::OpenPopup("AddComponent");
        }
        ImGui::SetItemTooltip("Add Component");
        ImGui::PopStyleColor(2);

        if (ImGui::BeginPopup("AddComponent")) {
            if (ImGui::MenuItem("Identifier")) {
                registry.emplace<core::components::Identifier>(entity);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Orthographic Projection")) {
                registry.emplace<core::components::OrthographicProjection>(
                    entity);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Transform")) {
                registry.emplace<core::components::Transform>(entity);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Texture")) {
                registry.emplace<core::components::Texture>(entity);
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::SeparatorText("Identification");

        // identification component
        char buffer[32];
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, identification.Name.c_str());
        if (ImGui::InputText("Tag##Tag", buffer, sizeof(buffer))) {
            identification.Name = std::string(buffer);
        }
        ImGui::Text("%s", identification.Uuid.to_string().c_str());

        ImGui::SeparatorText("Components");

        if (ImGui::BeginTable("components", 2)) {
            ImGui::TableSetupColumn("component",
                                    ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("remove", ImGuiTableColumnFlags_WidthFixed);

            // transform
            auto transform =
                registry.try_get<core::components::Transform>(entity);
            if (transform != nullptr) {
                ImGui::PushID(std::string("transform##")
                                  .append(identification.Uuid.to_string())
                                  .c_str());

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                if (ImGui::CollapsingHeader("Transform")) {
                    ImGui::PushStyleColor(ImGuiCol_FrameBg,
                                          ImVec4(40.0f / 255.0f, 42.0f / 255.0f,
                                                 54.0f / 255.0f, 1.0f));

                    glm::vec3 translation = transform->get_translation();
                    if (ImGui::DragFloat3(
                            "Translation", math::value_ptr(translation), 1.0f,
                            -std::numeric_limits<float>::max(),
                            std::numeric_limits<float>::max(), "%0.f")) {
                        transform->set_translation(translation);
                    }

                    glm::vec3 scale = transform->get_scale();
                    if (ImGui::DragFloat3("Scale", math::value_ptr(scale), 1.0f,
                                          -std::numeric_limits<float>::max(),
                                          std::numeric_limits<float>::max(),
                                          "%0.f")) {
                        transform->set_scale(scale);
                    }

                    glm::vec3 degrees =
                        math::degrees(transform->get_rotation());
                    if (ImGui::DragFloat3(
                            "Rotation", math::value_ptr(degrees), 1.0f,
                            -std::numeric_limits<float>::max(),
                            std::numeric_limits<float>::max(), "%0.f",
                            ImGuiSliderFlags_NoRoundToFormat)) {
                        transform->set_rotation(math::radians(degrees));
                    }

                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                ImGui::PushFont(g_rrContext->Fonts.Icon);
                if (ImGui::Button(ICON_FA_TIMES)) {
                    registry.remove<core::components::Transform>(entity);
                }
                ImGui::PopFont();

                ImGui::PopID();
            }

            // texture
            auto texture = registry.try_get<core::components::Texture>(entity);
            if (texture != nullptr) {
                ImGui::PushID(std::string("texture##")
                                  .append(identification.Uuid.to_string())
                                  .c_str());

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                if (ImGui::CollapsingHeader("Texture")) {
                    ImGui::PushStyleColor(ImGuiCol_FrameBg,
                                          ImVec4(40.0f / 255.0f, 42.0f / 255.0f,
                                                 54.0f / 255.0f, 1.0f));

                    char buffer[256];
                    memset(buffer, 0, sizeof(buffer));
                    strcpy(buffer, texture->TextureUUID.to_string().c_str());
                    if (ImGui::InputText("UUID", buffer, sizeof(buffer))) {
                        texture->TextureUUID = UUID(std::string(buffer));
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload =
                                ImGui::AcceptDragDropPayload(
                                    "TEXTURE_SPECIFICATION")) {
                            require(
                                payload->DataSize ==
                                    sizeof(graphics::ImageTextureSpecification),
                                "errororororo");
                            graphics::ImageTextureSpecification spec =
                                *(const graphics::ImageTextureSpecification*)
                                     payload->Data;

                            texture->TextureUUID = spec.Id;
                        }
                        if (const ImGuiPayload* payload =
                                ImGui::AcceptDragDropPayload(
                                    "SPRITE_SPECIFICATION")) {
                            require(payload->DataSize ==
                                        sizeof(graphics::SpriteSpecification),
                                    "errororororo");
                            graphics::SpriteSpecification spec =
                                *(const graphics::SpriteSpecification*)
                                     payload->Data;

                            texture->TextureUUID = spec.Id;
                        }
                        ImGui::EndDragDropTarget();
                    }

                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();

                ImGui::PushFont(g_rrContext->Fonts.Icon);
                if (ImGui::Button(ICON_FA_TIMES)) {
                    registry.remove<core::components::Texture>(entity);
                }
                ImGui::PopFont();

                ImGui::PopID();
            }

            // ortho projection
            auto orthoProjection =
                registry.try_get<core::components::OrthographicProjection>(
                    entity);
            if (orthoProjection != nullptr) {
                ImGui::PushID(std::string("ortho_projection##")
                                  .append(identification.Uuid.to_string())
                                  .c_str());

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                if (ImGui::CollapsingHeader("Orthographic Projection")) {
                    ImGui::PushStyleColor(ImGuiCol_FrameBg,
                                          ImVec4(40.0f / 255.0f, 42.0f / 255.0f,
                                                 54.0f / 255.0f, 1.0f));

                    float size = orthoProjection->get_size();
                    if (ImGui::DragFloat("Size", &size, 0.05f, 0.01f, 50.0f,
                                         "%.3f",
                                         ImGuiSliderFlags_AlwaysClamp)) {
                        orthoProjection->set_size(size);
                    }

                    float near = orthoProjection->get_near_clip();
                    if (ImGui::DragFloat("Near Clip", &near, 0.05f)) {
                        orthoProjection->set_near_clip(near);
                    }

                    float far = orthoProjection->get_far_clip();
                    if (ImGui::DragFloat("Far Clip", &far, 0.05f)) {
                        orthoProjection->set_far_clip(far);
                    }

                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();

                ImGui::PushFont(g_rrContext->Fonts.Icon);
                if (ImGui::Button(ICON_FA_TIMES)) {
                    registry.remove<core::components::OrthographicProjection>(
                        entity);
                }
                ImGui::PopFont();

                ImGui::PopID();
            }

            ImGui::EndTable();
        }

        ImGui::PopStyleVar(1);

        ImGui::EndTabItem();
    }

    ImGui::PopID();
}

void show_property_tab_texture(const UUID& id,
                               core::AssetManager& assetManager,
                               bool& open,
                               ImGuiSelectableFlags flags) {
    auto& spec = assetManager.get_image_texture_database()[id];

    if (ImGui::BeginTabItem(spec.Name.c_str(), &open, flags)) {
        char buffer[32];
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, spec.Name.c_str());
        if (ImGui::InputText("Name##Name", buffer, sizeof(buffer))) {
            spec.Name = std::string(buffer);
        }

        ImGui::Text("%s", spec.Id.to_string().c_str());

        ImGui::Separator();

        // image
        auto texture = assetManager.get_texture(id).lock();
        size_t textureId = texture->get_renderer_id();
        auto textureSize = texture->get_size();
        ImVec2 uv_min = ImVec2(0.0f, 0.0f);  // Top-left
        ImVec2 uv_max = ImVec2(1.0f, 1.0f);  // Lower-right
        ImVec2 region = ImGui::GetContentRegionAvail();
        float ar = (float)textureSize.x / (float)textureSize.y;
        textureSize.x = region.x;
        textureSize.y = region.x * ar;
        ImGui::Image((ImTextureID)textureId,
                     ImVec2(textureSize.x, textureSize.y), uv_min, uv_max);

        ImGui::EndTabItem();
    }
}

void show_property_tab_sprite(const UUID& id,
                              core::AssetManager& assetManager,
                              bool& open,
                              ImGuiSelectableFlags flags) {
    auto& spec = assetManager.get_sprite_database()[id];

    if (ImGui::BeginTabItem(spec.Name.c_str(), &open, flags)) {
        char buffer[32];
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, spec.Name.c_str());
        if (ImGui::InputText("Name##Name", buffer, sizeof(buffer))) {
            spec.Name = std::string(buffer);
        }

        ImGui::Text("%s", spec.Id.to_string().c_str());

        ImGui::Separator();

        auto spritePtr = assetManager.get_texture(spec.Id).lock();
        size_t textureId = spritePtr->get_renderer_id();
        auto textureSize = spritePtr->get_size();
        // top-left
        ImVec2 uv_min = ImVec2(spritePtr->get_texture_coords()[0].x,
                               spritePtr->get_texture_coords()[0].y);
        // bot-right
        ImVec2 uv_max = ImVec2(spritePtr->get_texture_coords()[2].x,
                               spritePtr->get_texture_coords()[2].y);
        ImVec2 region = ImGui::GetContentRegionAvail();
        float ar = (float)textureSize.x / (float)textureSize.y;
        textureSize.x = region.x;
        textureSize.y = region.x * ar;
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
        ImGui::Image((void*)textureId, ImVec2(textureSize.x, textureSize.y),
                     uv_min, uv_max);
        ImGui::PopStyleVar();

        ImGui::EndTabItem();
    }
}

void show_property_tab_font(const UUID& id,
                            core::AssetManager& assetManager,
                            bool& open,
                            ImGuiSelectableFlags flags) {
    auto& spec = assetManager.get_font_database()[id];
    if (ImGui::BeginTabItem(spec.Name.c_str(), &open, flags)) {
        char buffer[32];
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, spec.Name.c_str());
        if (ImGui::InputText("Name##Name", buffer, sizeof(buffer))) {
            spec.Name = std::string(buffer);
        }

        ImGui::Text("%s", spec.Id.to_string().c_str());

        ImGui::Separator();

        if (ImGui::CollapsingHeader("Font Metrics")) {
            if (ImGui::BeginTable("Font Metrics", 2)) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("em size");
                ImGui::TableNextColumn();
                ImGui::Text("%f", spec.FontMetrics.emSize);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("ascender");
                ImGui::TableNextColumn();
                ImGui::Text("%f", spec.FontMetrics.ascender);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("descender");
                ImGui::TableNextColumn();
                ImGui::Text("%f", spec.FontMetrics.descender);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("line height");
                ImGui::TableNextColumn();
                ImGui::Text("%f", spec.FontMetrics.lineHeight);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("underline Y");
                ImGui::TableNextColumn();
                ImGui::Text("%f", spec.FontMetrics.underlineY);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("underline thickness");
                ImGui::TableNextColumn();
                ImGui::Text("%f", spec.FontMetrics.underlineThickness);

                ImGui::EndTable();
            }
        }

        if (ImGui::CollapsingHeader("Atlas Metrics")) {
            if (ImGui::BeginTable("Atlas Metrics", 2)) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("atlas type");
                ImGui::TableNextColumn();
                {
                    auto name = magic_enum::enum_name(spec.AtlasMetrics.type);
                    std::string nameStr = std::string{name};
                    std::transform(nameStr.begin(), nameStr.end(),
                                   nameStr.begin(),
                                   [](unsigned char c) {
                                       return std::tolower(c);
                                   }  // correct
                    );
                    ImGui::Text("%s", nameStr.c_str());
                }

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("distance range");
                ImGui::TableNextColumn();
                ImGui::Text("%f", spec.AtlasMetrics.distanceRange);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("size");
                ImGui::TableNextColumn();
                ImGui::Text("%f", spec.AtlasMetrics.size);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("width");
                ImGui::TableNextColumn();
                ImGui::Text("%d", spec.AtlasMetrics.width);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("height");
                ImGui::TableNextColumn();
                ImGui::Text("%d", spec.AtlasMetrics.height);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("y origin");
                ImGui::TableNextColumn();
                {
                    auto name =
                        magic_enum::enum_name(spec.AtlasMetrics.yOrigin);
                    std::string nameStr = std::string{name};
                    std::transform(nameStr.begin(), nameStr.end(),
                                   nameStr.begin(),
                                   [](unsigned char c) {
                                       return std::tolower(c);
                                   }  // correct
                    );
                    ImGui::Text("%s", nameStr.c_str());
                }

                ImGui::EndTable();
            }
        }

        if (ImGui::CollapsingHeader("Glyph Metrics")) {
            if (ImGui::BeginTable("Glyph Metrics", 3)) {
                ImGui::TableSetupColumn("codepoint");
                ImGui::TableSetupColumn("character");
                ImGui::TableSetupColumn("advance");
                ImGui::TableHeadersRow();

                for (auto& [codepoint, metrics] : spec.GlyphMetrics) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", codepoint);

                    ImGui::TableNextColumn();
                    ImGui::Text("%c", static_cast<char>(codepoint));

                    ImGui::TableNextColumn();
                    ImGui::Text("%f", metrics.advance);
                }

                ImGui::EndTable();
            }
        }

        if (ImGui::CollapsingHeader("Atlas Preview")) {
            // image
            auto texture = assetManager.get_texture(id).lock();
            size_t textureId = texture->get_renderer_id();
            auto textureSize = texture->get_size();
            ImVec2 uv_min = ImVec2(0.0f, 0.0f);  // Top-left
            ImVec2 uv_max = ImVec2(1.0f, 1.0f);  // Lower-right
            ImVec2 region = ImGui::GetContentRegionAvail();
            float ar = (float)textureSize.x / (float)textureSize.y;
            textureSize.x = region.x;
            textureSize.y = region.x * ar;
            ImGui::Image((ImTextureID)textureId,
                         ImVec2(textureSize.x, textureSize.y), uv_min, uv_max);
        }

        ImGui::EndTabItem();
    }
}

void show_property_tab_shader() {
    // TODO implement
}

void show_file_browser(const system::Path& rootDir) {
    if (!g_rrContext->ShowFileBrowser)
        return;

    static system::Path currentDir = rootDir;

    if (ImGui::Begin("File Browser", &g_rrContext->ShowFileBrowser)) {
        ImGui::BeginDisabled(currentDir == rootDir);
        if (ImGui::Button("back")) {
            currentDir = currentDir.parent_path();
        }
        ImGui::EndDisabled();

        ImGui::Separator();

        std::vector<system::directory_entry> entries;

        for (auto& p : system::directory_iterator(currentDir)) {
            if (p.is_directory()) {
                entries.push_back(p);
            }
        }

        for (auto& p : system::directory_iterator(currentDir)) {
            if (p.is_regular_file()) {
                entries.push_back(p);
            }
        }

        ImVec2 availableRegion = ImGui::GetContentRegionAvail();

        int rowAmount = std::floor(availableRegion.x /
                                   (g_rrContext->Style.ThumbnailSize.x +
                                    g_rrContext->Style.ThumbnailPadding.x * 2));

        ImGuiTableFlags flags = ImGuiTableFlags_PadOuterX;
        ImGui::PushStyleVar(
            ImGuiStyleVar_CellPadding,
            ImVec2(g_rrContext->Style.ThumbnailPadding.x,
                   g_rrContext->Style.ThumbnailPadding.y / 2.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.5, 0.5, 0.5, 0.8));
        if (ImGui::BeginTable("contents", std::clamp(rowAmount, 1, 511),
                              flags)) {
            for (auto& p : entries) {
                ImGui::TableNextColumn();
                system::Path path = p.path();
                std::string relativeRootName =
                    system::relative_root_name(path, rootDir);
                size_t image;
                if (p.is_directory()) {
                    image = g_rrContext->Icons.Folder->get_renderer_id();
                    ImGui::ImageButton((ImTextureID)image,
                                       g_rrContext->Style.ThumbnailSize, {0, 0},
                                       {1, 1});

                    if (ImGui::IsItemHovered() &&
                        ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        currentDir = path;
                    }
                } else if (p.is_regular_file()) {
                    image = g_rrContext->Icons.File->get_renderer_id();
                    ImGui::ImageButton((ImTextureID)image,
                                       g_rrContext->Style.ThumbnailSize, {0, 0},
                                       {1, 1});

                    if (relativeRootName == "scenes") {
                        const char* pathStr = path.c_str();
                        if (ImGui::BeginDragDropSource()) {
                            ImGui::SetDragDropPayload(
                                "CONTENT_BROWSER_SCENE_PATH", pathStr,
                                strlen(pathStr) + sizeof(char));

                            ImGui::EndDragDropSource();
                        }
                    }
                }
                ImGui::TextWrapped("%s", path.filename().c_str());
            }
            ImGui::EndTable();
        }
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();
    }
    ImGui::End();
}

void show_scene_viewport(
    core::Scene& scene,
    std::function<void()> drawSceneFn,
    std::function<bool(const system::Path&)> changeSceneFn) {
    if (!g_rrContext->ShowSceneViewport)
        return;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    if (ImGui::Begin("Scene Viewport", &g_rrContext->ShowSceneViewport,
                     ImGuiWindowFlags_NoScrollbar)) {
        ImVec2 size = ImGui::GetContentRegionAvail();
        math::ivec2 newSize = {size.x, size.y};

        ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
        ImVec2 mousePosition = ImGui::GetMousePos();
        mousePosition.x -= cursorScreenPos.x;
        mousePosition.y -= cursorScreenPos.y;
        mousePosition.y = size.y - mousePosition.y;

        auto& framebuffer = scene.get_framebuffer();

        if (framebuffer.get_size() != newSize) {
            scene.set_viewport_size(newSize);
            g_rrContext->SceneViewportSize = newSize;
        }

        drawSceneFn();

        size_t texID = framebuffer.get_color_attachment_renderer_id();
        ImGui::Image((void*)texID, ImVec2{size.x, size.y}, ImVec2{0, 1},
                     ImVec2{1, 0});

        // get entity id at mouse position from attachement
        if (mousePosition.x >= 0 && mousePosition.y >= 0 &&
            mousePosition.x <= size.x && mousePosition.y <= size.y &&
            ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            framebuffer.bind();
            uint32_t selectedEntity =
                framebuffer
                    .read_pixel_data(1, {mousePosition.x, mousePosition.y})
                    .UnsignedInt;
            framebuffer.unbind();

            if (selectedEntity != (uint32_t)entt::null) {
                g_rrContext->SelectedEntity = (entt::entity)selectedEntity;
                auto& registry = scene.get_registry();
                auto& identification =
                    registry.get<core::components::Identifier>(
                        (entt::entity)selectedEntity);

                ui::push_property_tab(PropertyTab(
                    PropertyTabType::Entity, {(entt::entity)selectedEntity},
                    identification.Uuid));
            }
        }

        // target for scene file drag and drop
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(
                    "CONTENT_BROWSER_SCENE_PATH")) {
                system::Path path = system::Path((const char*)payload->Data);
                changeSceneFn(path);
            }
            ImGui::EndDragDropTarget();
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

/////////////////////////////
//  ITEMS
void show_toasts() {
    using namespace std::chrono;

    const ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
    uint64_t now =
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch())
            .count();

    float positionY = 0.0f;

    for (auto iter = g_rrContext->Toasts.begin();
         iter != g_rrContext->Toasts.end();) {
        float elapsedTime = now - iter->StartTime;
        const auto& style = g_rrContext->Style.ToastTypeStyle[iter->Type];

        ToastPhase currentPhase = ToastPhase::Finished;
        if (elapsedTime <= style.FadeInTime) {
            currentPhase = ToastPhase::FadeIn;
        } else if (elapsedTime <= style.DisplayTime + style.FadeInTime) {
            currentPhase = ToastPhase::Display;
        } else if (elapsedTime <=
                   style.DisplayTime + style.FadeInTime + style.FadeOutTime) {
            currentPhase = ToastPhase::FadeOut;
        }

        float alpha = 1.0f;
        if (currentPhase == ToastPhase::FadeIn) {
            alpha = std::clamp(elapsedTime / style.FadeInTime, 0.0f, 1.0f);
        } else if (currentPhase == ToastPhase::FadeOut) {
            alpha = std::clamp(
                1.0f - (elapsedTime - style.FadeInTime - style.DisplayTime) /
                           style.FadeOutTime,
                0.0f, 1.0f);
        } else if (currentPhase == ToastPhase::Finished) {
            alpha = 0.0f;
        }

        std::string defaultTitle;
        switch (iter->Type) {
            case ToastType::Info:
                defaultTitle = "Info";
                break;
            case ToastType::Warning:
                defaultTitle = "Warning";
                break;
            case ToastType::Error:
                defaultTitle = "Error";
                break;
            case ToastType::Success:
                defaultTitle = "Success";
                break;
            default:
                break;
        }

        ImGuiWindowFlags windowFlags =
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoDecoration |
            // ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoNav |
            // ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_Tooltip;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,
                            g_rrContext->Style.ToastBorderRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_PopupBg, style.BackgroundColor);
        ImGui::SetNextWindowSizeConstraints(
            ImVec2(g_rrContext->Style.ToastMinWidth, -1.0f),
            ImVec2(g_rrContext->Style.ToastMaxWidth, -1.0f));
        ImGui::SetNextWindowBgAlpha(alpha);

        switch (g_rrContext->Style.ToastViewportPosition) {
            case ToastPosition::BottomRight:
                ImGui::SetNextWindowPos(
                    ImVec2(viewportSize.x - g_rrContext->Style.ToastPadding.x,
                           viewportSize.y - g_rrContext->Style.ToastPadding.y -
                               positionY),
                    ImGuiCond_Always, ImVec2(1.0f, 1.0f));
                break;
            case ToastPosition::TopRight:
                ImGui::SetNextWindowPos(
                    ImVec2(viewportSize.x - g_rrContext->Style.ToastPadding.x,
                           g_rrContext->Style.ToastPadding.y + positionY),
                    ImGuiCond_Always, ImVec2(1.0f, 0.0f));
                break;
            case ToastPosition::BottomLeft:
                ImGui::SetNextWindowPos(
                    ImVec2(g_rrContext->Style.ToastPadding.x,
                           viewportSize.y - g_rrContext->Style.ToastPadding.y -
                               positionY),
                    ImGuiCond_Always, ImVec2(0.0f, 1.0f));
                break;
            case ToastPosition::TopLeft:
                ImGui::SetNextWindowPos(
                    ImVec2(g_rrContext->Style.ToastPadding.x,
                           g_rrContext->Style.ToastPadding.y + positionY),
                    ImGuiCond_Always, ImVec2(0.0f, 0.0f));
                break;
            default:
                ImGui::SetNextWindowPos(
                    ImVec2(viewportSize.x - g_rrContext->Style.ToastPadding.x,
                           viewportSize.y - g_rrContext->Style.ToastPadding.y -
                               positionY),
                    ImGuiCond_Always, ImVec2(1.0f, 0.0f));
                break;
        }

        // ImGui::PushID(iter->Id.to_string().c_str());
        if (ImGui::Begin(iter->Id.to_string().c_str(), NULL, windowFlags)) {
            {
                // TODO padding
                ImGui::PushTextWrapPos(
                    g_rrContext->Style
                        .ToastMaxWidth);  // We want to support multi-line text,
                                          // this will wrap the text after 1/3
                                          // of the screen width

                bool was_title_rendered = false;

                if (style.IconCodepoint.has_value()) {
                    if (g_rrContext->Style.ToastIconFont != nullptr) {
                        ImGui::PushFont(g_rrContext->Style.ToastIconFont);
                    }

                    auto& color = style.IconColor;
                    ImGui::TextColored(ImVec4(color.x, color.y, color.z, alpha),
                                       "%s", style.IconCodepoint.value());
                    was_title_rendered = true;

                    if (g_rrContext->Style.ToastIconFont != nullptr) {
                        ImGui::PopFont();
                    }
                }

                if (g_rrContext->Style.ToastContentFont != nullptr)
                    ImGui::PushFont(g_rrContext->Style.ToastContentFont);

                if (!iter->Title.empty()) {
                    if (style.IconCodepoint.has_value())
                        ImGui::SameLine();

                    auto& color = style.TitleColor;
                    ImGui::TextColored(
                        ImVec4(color.x, color.y, color.z, alpha), "%s",
                        iter->Title.c_str());  // Render title text
                    was_title_rendered = true;
                } else if (!defaultTitle.empty()) {
                    if (style.IconCodepoint.has_value())
                        ImGui::SameLine();

                    auto& color = style.TitleColor;
                    ImGui::TextColored(
                        ImVec4(color.x, color.y, color.z, alpha), "%s",
                        defaultTitle.c_str());  // Render default title text
                                                // (ImGuiToastType_Success ->
                                                // "Success", etc...)
                    was_title_rendered = true;
                }

                if (was_title_rendered && !iter->Message.empty()) {
                    ImGui::SetCursorPosY(
                        ImGui::GetCursorPosY() +
                        5.f);  // Must be a better way to do this!!!!
                }

                if (!iter->Message.empty()) {
                    if (was_title_rendered) {
                        if (g_rrContext->Style.ToastContentSeparator) {
                            auto& color = style.SeparatorColor;
                            ImGui::PushStyleColor(
                                ImGuiCol_Separator,
                                ImVec4(color.x, color.y, color.z, alpha));
                            ImGui::Separator();
                            ImGui::PopStyleColor(1);
                        }
                    }

                    auto& color = style.MessageColor;
                    ImGui::TextColored(
                        ImVec4(color.x, color.y, color.z, alpha), "%s",
                        iter->Message.c_str());  // Render content text
                }

                if (g_rrContext->Style.ToastContentFont != nullptr)
                    ImGui::PopFont();

                ImGui::PopTextWrapPos();
            }

            positionY +=
                ImGui::GetWindowHeight() + g_rrContext->Style.ToastPadding.y;

            // hardcoding hovered trigger, so toasts can be
            // closed when modals are active
            // TODO toast currently gets dismissed on press rather than release
            ImVec2 mPos = ImGui::GetMousePos();
            ImVec2 wPos = ImGui::GetWindowPos();
            ImVec2 wSize = ImGui::GetWindowSize();
            ImVec2 wMin = ImVec2(wPos.x, wPos.y);
            ImVec2 wMax = ImVec2(wPos.x + wSize.x, wPos.y + wSize.y);
            if (mPos.x >= wMin.x && mPos.x <= wMax.x && mPos.y >= wMin.y &&
                mPos.y <= wMax.y &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Left, false)) {
                currentPhase = ToastPhase::Finished;
            }
        }
        ImGui::End();

        ImGui::PopStyleColor(1);
        ImGui::PopStyleVar(2);

        if (currentPhase == ToastPhase::Finished) {
            ImGui::CloseCurrentPopup();
            iter = g_rrContext->Toasts.erase(iter);
        } else {
            iter++;
        }
    }
}

void push_toast(const Toast& toast) {
    g_rrContext->Toasts.push_back(toast);
}

}  // namespace rr::editor::ui
