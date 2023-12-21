#ifndef RR_EDITOR_UI_ITEMS_COMPONENTS_H
#define RR_EDITOR_UI_ITEMS_COMPONENTS_H

#include "rurouni/core/components.hpp"

#include <imgui/imgui.h>
#include <entt/entt.hpp>
#include <string>

namespace rr::editor {

/////////////////////////////////////////////////////////////
///////////////////  COMPONENTS  ////////////////////////////
/////////////////////////////////////////////////////////////
void draw_component_identification(core::components::Identifier& component);
void draw_component_texture(core::components::Texture& component);
void draw_component_ortho_projection(
    core::components::OrthographicProjection& component);
void draw_component_transform(core::components::Transform& component);

/////////////////////////////////////////////////////////////
///////////////  COMPONENT TEMPLATE  ////////////////////////
/////////////////////////////////////////////////////////////
template <typename T, typename Func>
void draw_component(entt::registry& registry,
                    entt::entity entityId,
                    const std::string& label,
                    bool removable,
                    const Func& func) {
    ImGui::PushID(label.c_str());
    if (registry.any_of<T>(entityId)) {
        auto& component = registry.get<T>(entityId);

        bool removeComponent = false;
        if (removable) {
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4{0.8f, 0.15f, 0.2f, 1.0f});
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4{0.9f, 0.25f, 0.3f, 1.0f});
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                  ImVec4{0.7f, 0.15f, 0.1f, 1.0f});
            if (ImGui::Button("X", ImVec2{ImGui::GetFrameHeight(), 0})) {
                removeComponent = true;
            }
            ImGui::PopStyleColor(3);
            ImGui::SameLine();
        }

        // if constexpr (requires { T::Active; }) {
        //   ImGui::Checkbox("", &component.Active);
        //   ImGui::SameLine();
        // }

        if (ImGui::CollapsingHeader(label.c_str())) {
            ImGui::PushStyleColor(
                ImGuiCol_FrameBg,
                ImVec4(40.0f / 255.0f, 42.0f / 255.0f, 54.0f / 255.0f, 1.0f));
            func(component);
            ImGui::PopStyleColor();
        }

        if (removeComponent) {
            registry.remove<T>(entityId);
        }
    }
    ImGui::PopID();
}

}  // namespace rr::editor

#endif  // !RR_EDITOR_UI_ITEMS_COMPONENTS_H
