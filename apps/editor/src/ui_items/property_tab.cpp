#include "rurouni/editor/ui_items/property_tab.hpp"
// #include "rurouni/editor/ui_items/components.hpp"

// #include "rurouni/core/components.hpp"

#include <imgui/imgui.h>

namespace rr::editor::ui {

/*
void EntityProperty::draw(entt::registry& registry,
                      core::AssetManager& assetManager) {
// header buttons
if (ImGui::Button("Add")) {
    ImGui::OpenPopup("AddComponent");
}
draw_component_add_popup(registry, m_Entity);

ImGui::Separator();

// identification component
auto& identification = registry.get<components::Identifier>(m_Entity);
imgui_draw_component_identification(identification);

ImGui::Separator();

// Components
imgui_draw_component<components::Transform>(
    registry, m_Entity, "Transform", false, imgui_draw_component_transform);
imgui_draw_component<components::Texture>(
    registry, m_Entity, "Texture", true, imgui_draw_component_texture);
imgui_draw_component<components::Sprite>(registry, m_Entity, "Sprite", true,
                                         imgui_draw_component_sprite);
imgui_draw_component<components::OrthographicProjection>(
    registry, m_Entity, "Orthigraphic Projection", true,
    imgui_draw_component_ortho_projection);
}

void EntityProperty::draw_component_add_popup(entt::registry& registry,
                                          entt::entity entityId) {
if (ImGui::BeginPopup("AddComponent")) {
    if (ImGui::MenuItem("Identifier")) {
        registry.emplace<components::Identifier>(entityId);
        ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("Orthographic Projection")) {
        registry.emplace<components::OrthographicProjection>(entityId);
        ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("Transform")) {
        registry.emplace<components::Transform>(entityId);
        ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("Texture")) {
        registry.emplace<components::Texture>(entityId);
        ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("Sprite")) {
        registry.emplace<components::Sprite>(entityId);
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}
}

const types::UUID& EntityProperty::get_id() const {
return m_Id;
}
*/
const UUID& TextureProperty::get_id() const {
    return m_Id;
}
const UUID& SpriteProperty::get_id() const {
    return m_SpriteId;
}

void TextureProperty::draw(entt::registry& registry,
                           core::AssetManager& assetManager) {
    auto& spec = assetManager.get_texture_registry()[m_Id];

    char buffer[32];
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, spec.Name.c_str());
    if (ImGui::InputText("Name##Name", buffer, sizeof(buffer))) {
        spec.Name = std::string(buffer);
    }

    ImGui::Text("%s", spec.Id.to_string().c_str());

    ImGui::Separator();

    // image
    auto texture = assetManager.get_texture(m_Id).lock();
    size_t textureId = texture->get_renderer_id().value();
    auto textureSize = texture->get_size();
    ImVec2 uv_min = ImVec2(0.0f, 0.0f);  // Top-left
    ImVec2 uv_max = ImVec2(1.0f, 1.0f);  // Lower-right
    ImVec2 region = ImGui::GetContentRegionAvail();
    float ar = (float)textureSize.x / (float)textureSize.y;
    textureSize.x = region.x;
    textureSize.y = region.x * ar;
    ImGui::Image((void*)textureId, ImVec2(textureSize.x, textureSize.y), uv_min,
                 uv_max);
}

void SpriteProperty::draw(entt::registry& registry,
                          core::AssetManager& assetManager) {
    auto& spec = assetManager.get_sprite_registry()[m_SpriteId];

    char buffer[32];
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, spec.Name.c_str());
    if (ImGui::InputText("Name##Name", buffer, sizeof(buffer))) {
        spec.Name = std::string(buffer);
    }

    ImGui::Text("%s", spec.Id.to_string().c_str());

    ImGui::Separator();

    auto spritePtr = assetManager.get_texture(spec.Id).lock();
    size_t textureId = spritePtr->get_renderer_id().value();
    auto textureSize = spritePtr->get_size();
    // top-left
    ImVec2 uv_min = ImVec2(spritePtr->get_uv_coords()[0].x,
                           spritePtr->get_uv_coords()[0].y);
    // bot-right
    ImVec2 uv_max = ImVec2(spritePtr->get_uv_coords()[2].x,
                           spritePtr->get_uv_coords()[2].y);
    ImVec2 region = ImGui::GetContentRegionAvail();
    float ar = (float)textureSize.x / (float)textureSize.y;
    textureSize.x = region.x;
    textureSize.y = region.x * ar;
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    ImGui::Image((void*)textureId, ImVec2(textureSize.x, textureSize.y), uv_min,
                 uv_max);
    ImGui::PopStyleVar();
}

}  // namespace rr::editor::ui
