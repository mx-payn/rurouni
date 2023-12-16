#include "rurouni/editor/ui.hpp"

#include "rurouni/editor.hpp"
#include "rurouni/event/application_event.hpp"
#include "rurouni/event/event_system.hpp"
#include "rurouni/graphics/window.hpp"
#include "rurouni/math.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/types/uuid.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <fstream>
#include <memory>
#include <optional>

namespace rr::editor::ui {

void init(graphics::Window& window,
          const system::Path& sharedDataDir,
          const system::Path& userDataDir,
          const system::Path& userConfigDir) {
    // create context;
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // configure io flags
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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
    io.Fonts->AddFontFromFileTTF(
        (sharedDataDir / "fonts/caskaydia_code_mono_regular.ttf").c_str(),
        15 * contentScale.x);

    // init additional fonts
    // s_Fonts.FontAwesome = io.Fonts->AddFontFromFileTTF(
    //     (sharedDataDir / "fonts/font_awesome_regular.otf").c_str(),
    //     15 * contentScale.x);

    // init icons

    // styling
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // graphics context initialization
    const char* glsl_version = "#version 410";
    ImGui_ImplGlfw_InitForOpenGL(window.get_native_window(), true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void terminate() {
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

void draw_dockspace(UIState& state) {
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

    // draw_menu_bar(state);

    ImGui::End();
}

// void draw_menu_bar(State& state) {
//     if (ImGui::BeginMenuBar()) {
//         if (ImGui::BeginMenu("View")) {
//             ImGui::MenuItem("Scene Viewport", NULL,
//                             &state.ShowPanelSceneViewport);
//             ImGui::MenuItem("Properties", NULL, &state.ShowPanelProperties);
//             ImGui::MenuItem("Scene", NULL, &state.ShowPanelScene);
//             ImGui::MenuItem("Assets", NULL, &state.ShowPanelAssets);
//             ImGui::MenuItem("Project", NULL, &state.ShowPanelProject);
//             ImGui::MenuItem("ImGui Demo", NULL, &state.ShowImGuiDemoWindow);
//
//             ImGui::EndMenu();
//         }
//
//         ImGui::EndMenuBar();
//     }
// }

}  // namespace rr::editor::ui
