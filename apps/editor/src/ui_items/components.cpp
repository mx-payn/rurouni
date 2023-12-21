#include "rurouni/editor/ui_items/components.hpp"
#include "rurouni/editor/logger.hpp"

#include "rurouni/core/asset_manager.hpp"
#include "rurouni/core/components.hpp"
#include "rurouni/math.hpp"

#include <imgui/imgui.h>

namespace rr::editor {

void draw_component_identification(core::components::Identifier& component) {
    // Name
    char buffer[32];
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, component.Name.c_str());
    if (ImGui::InputText("Tag##Tag", buffer, sizeof(buffer))) {
        component.Name = std::string(buffer);
    }

    // UUID
    ImGui::Text("%s", component.Uuid.to_string().c_str());
}

void draw_component_texture(core::components::Texture& component) {
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload =
                ImGui::AcceptDragDropPayload("TEXTURE_SPECIFICATION")) {
            require(payload->DataSize == sizeof(core::TextureSpecification),
                    "errororororo");
            core::TextureSpecification spec =
                *(const core::TextureSpecification*)payload->Data;

            component.TextureUUID = spec.Id;
        }
        ImGui::EndDragDropTarget();
    }

    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, component.TextureUUID.to_string().c_str());
    if (ImGui::InputText("UUID", buffer, sizeof(buffer))) {
        component.TextureUUID = UUID(std::string(buffer));
    }
}

void draw_component_ortho_projection(
    core::components::OrthographicProjection& component) {
    float size = component.get_size();
    if (ImGui::DragFloat("Size", &size, 0.05f, 0.01f, 50.0f, "%.3f",
                         ImGuiSliderFlags_AlwaysClamp)) {
        component.set_size(size);
    }

    float near = component.get_near_clip();
    if (ImGui::DragFloat("Near Clip", &near, 0.05f)) {
        component.set_near_clip(near);
    }

    float far = component.get_far_clip();
    if (ImGui::DragFloat("Far Clip", &far, 0.05f))
        component.set_far_clip(far);
}

void draw_component_transform(core::components::Transform& component) {
    glm::vec3 translation = component.get_translation();
    if (ImGui::DragFloat3("Translation", math::value_ptr(translation),
                          0.125f)) {
        component.set_translation(translation);
    }

    glm::vec3 scale = component.get_scale();
    if (ImGui::DragFloat3("Scale", math::value_ptr(scale), 0.125f)) {
        component.set_scale(scale);
    }

    glm::vec3 degrees = {math::degrees(component.get_rotation().x),
                         math::degrees(component.get_rotation().y),
                         math::degrees(component.get_rotation().z)};
    if (ImGui::DragFloat3("Rotation", math::value_ptr(degrees), 0.2f, -360.0f,
                          360.0f, "%1.f", ImGuiSliderFlags_AlwaysClamp)) {
        component.set_rotation({math::radians(degrees[0]),
                                math::radians(degrees[1]),
                                math::radians(degrees[2])});
    }
}

}  // namespace rr::editor
