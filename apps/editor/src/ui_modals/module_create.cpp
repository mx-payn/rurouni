#include "rurouni/editor/ui_modals/module_create.hpp"

#include "rurouni/system/command.hpp"

#include <imgui/imgui.h>

namespace rr::editor::ui {

void ModuleCreateModal::draw(
    UIState& state,
    std::function<void(const system::Path&, const std::string&)> createFunc) {
    if (!state.ShowModuleCreateModal)
        return;

    ImGui::OpenPopup("Create Module");

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Create Module", &state.ShowModuleCreateModal,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        static char name[32] = "";
        ImGui::InputText("name", name, 32);

        static char path[256] = "";
        ImGui::InputText("path", path, 256);

        ImGui::SameLine();

        if (ImGui::Button("Open")) {
            std::string result =
                system::execute_command("zenity --file-selection --directory");
            result.copy(path, result.size());
        }

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            createFunc(path, name);

            state.ShowModuleCreateModal = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            state.ShowModuleCreateModal = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
}  // namespace rr::editor::ui
