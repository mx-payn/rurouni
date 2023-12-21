// editor
#include "rurouni/editor/ui_panels/scene.hpp"
#include "rurouni/editor/state.hpp"
#include "rurouni/editor/ui_items/property_tab.hpp"
#include "rurouni/editor/ui_panels/properties.hpp"

// rurouni
#include "rurouni/core/components.hpp"
#include "rurouni/core/scene.hpp"
#include "rurouni/types/uuid.hpp"

// external
#include <imgui/imgui.h>
#include <entt/entt.hpp>

// std
#include <memory>
#include <string>

namespace rr::editor::ui {

void ScenePanel::draw(UIState& state,
                      core::Scene& scene,
                      std::function<void(const system::Path&)> changeSceneFn,
                      std::function<void(const system::Path&)> saveSceneFn) {
    ImGui::Begin("Scene", &state.ShowPanelScene);

    // dummy layer over whole content region to allow drag and drop
    // in whole window
    ImVec2 size = ImGui::GetContentRegionAvail();
    ImGui::SetNextItemAllowOverlap();
    ImGui::Dummy(size);
    ImVec2 pos = ImGui::GetItemRectMin();
    ImGui::SetCursorScreenPos(pos);

    // drag and drop scene file
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload =
                ImGui::AcceptDragDropPayload("CONTENT_BROWSER_SCENE_PATH")) {
            system::Path path = system::Path((const char*)payload->Data);

            changeSceneFn(path);
        }
        ImGui::EndDragDropTarget();
    }

    auto imageId = state.Icons.Save->get_renderer_id();
    if (imageId.has_value()) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.5, 0.5, 0.5, 0.8));

        size_t imageIdCast = static_cast<size_t>(imageId.value());
        if (ImGui::ImageButton(
                (ImTextureID)imageIdCast,
                {(float)state.IconSize.x, (float)state.IconSize.y}, {0, 1},
                {1, 0})) {
            saveSceneFn("");
        }

        ImGui::SetItemTooltip("Save scene");
        ImGui::PopStyleColor(2);
    } else {
        if (ImGui::Button("Save")) {
            saveSceneFn("");
        }
        ImGui::SetItemTooltip("Save scene");
    }

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
        view.each([&state, &registry](auto entityId) {
            draw_entity_node(state, registry, entityId);
        });

        // popup menu when right clicking on empty space
        int popupFlags =
            ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight;
        if (ImGui::BeginPopupContextWindow(0, popupFlags)) {
            // create a new entity
            if (ImGui::MenuItem("Create Entity")) {
                entt::entity entity = scene.create_entity("New Emptity");
            }

            ImGui::EndPopup();
        }

        ImGui::TreePop();
    }

    ImGui::End();
}

void ScenePanel::draw_entity_node(UIState& state,
                                  entt::registry& registry,
                                  entt::entity entityId) {
    // function variables
    auto& identifier = registry.get<core::components::Identifier>(entityId);

    ImGui::PushID(identifier.Uuid.to_string().c_str());

    // selection when left clicking an entity
    bool selected = state.m_SelectedEntity.has_value() &&
                    state.m_SelectedEntity.value() == entityId;
    if (ImGui::Selectable(identifier.Name.c_str(), selected)) {
        ui::PropertiesPanel::add_tab(std::make_unique<EntityProperty>(
            registry, entityId, identifier.Name));
    }

    // popup when right clicking on entity
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Delete Entity")) {
            registry.destroy(entityId);
        }

        ImGui::EndPopup();
    }

    ImGui::PopID();
}

}  // namespace rr::editor::ui
