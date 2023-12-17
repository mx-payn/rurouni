#include "rurouni/editor/ui_modals/error.hpp"

#include <imgui/imgui.h>

#include <string>
#include <vector>

namespace rr::editor::ui {

std::vector<ErrorModal::Error> ErrorModal::s_Errors =
    std::vector<ErrorModal::Error>();

void ErrorModal::draw() {
    if (s_Errors.empty())
        return;

    ImGui::OpenPopup("Error");

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Error", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        static int selected = 0;
        if (ImGui::BeginListBox("##list")) {
            for (int i = 0; i < s_Errors.size(); i++) {
                ImGui::PushID(std::to_string(i).c_str());
                const bool is_selected = (selected == i);
                if (ImGui::Selectable(s_Errors[i].Title.c_str(), is_selected))
                    selected = i;

                // Set the initial focus when opening the combo (scrolling +
                // keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();

                ImGui::PopID();
            }
            ImGui::EndListBox();
        }

        ImGui::SameLine();

        ImGui::BeginGroup();

        ImGui::TextWrapped("%s", s_Errors[selected].Message.c_str());
        ImGui::Spacing();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            s_Errors.clear();
            selected = 0;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Copy Message", ImVec2(120, 0))) {
            ImGui::LogToClipboard();
            ImGui::LogText("%s", s_Errors[selected].Message.c_str());
        }
        ImGui::EndGroup();
        ImGui::EndPopup();
    }
}

}  // namespace rr::editor::ui
