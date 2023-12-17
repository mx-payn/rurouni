#include "rurouni/editor/ui_panels/startup_splash.hpp"

#include "rurouni/event/application_event.hpp"
#include "rurouni/system/command.hpp"
#include "rurouni/system/filesystem.hpp"

#include <imgui/imgui.h>

namespace rr::editor::ui {

void StartupSplash::draw(UIState& state,
                         event::EventSystem& eventSystem,
                         std::unordered_map<UUID, ModuleHistoryItem>& history,
                         std::function<void(const system::Path&)> importFunc,
                         std::function<void(const UUID&)> openFunc) {
    // setup
    static bool open = true;
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::SetNextWindowPos(
        ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
        ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::Begin(
        "Rurouni Splash", &open,
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

    // history box
    static UUID selectedId;
    draw_history(history, selectedId);

    // new project button
    if (ImGui::Button("New")) {
        state.ShowModuleCreateModal = true;
    }

    ImGui::SameLine();

    // import button
    draw_import(importFunc);

    ImGui::SameLine();

    // open button
    ImGui::BeginDisabled(selectedId.is_null());
    if (ImGui::Button("Open")) {
        openFunc(selectedId);
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    // exit application
    draw_exit(eventSystem);

    ImGui::End();
}

void StartupSplash::draw_history(
    std::unordered_map<UUID, ModuleHistoryItem>& history,
    UUID& selectedId) {
    // TODO validation: does the project still exist?
    if (ImGui::BeginListBox("history")) {
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
}

void StartupSplash::draw_import(
    std::function<void(const system::Path&)> importFunc) {
    if (ImGui::Button("Import")) {
        // TODO make a modal to specify name and path
        // which would enable custom name when clashing
        auto result = system::execute_command("zenity --file-selection");
        auto path = system::Path(result);

        importFunc(path);
    }
}

void StartupSplash::draw_exit(event::EventSystem& eventSystem) {
    if (ImGui::Button("Exit")) {
        auto event = std::make_shared<event::ApplicationClose>();
        eventSystem.fire_event(event);
    }
}

}  // namespace rr::editor::ui
