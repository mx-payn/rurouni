#include "rurouni/editor/ui_panels/scene_viewport.hpp"

#include "entt/entity/entity.hpp"
#include "rurouni/core/components/identifier.hpp"
#include "rurouni/core/scene.hpp"
#include "rurouni/editor/logger.hpp"
#include "rurouni/editor/ui_panels/properties.hpp"

#include <imgui/imgui.h>

namespace rr::editor::ui {

uint32_t SceneViewportPanel::m_HoveredEntity = (uint32_t)entt::null;

void SceneViewportPanel::draw(
    UIState& state,
    core::Scene& scene,
    std::function<void()> drawSceneFn,
    std::function<void(const system::Path&)> changeSceneFn) {
    if (!state.ShowPanelSceneViewport)
        return;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Scene Viewport", &state.ShowPanelSceneViewport,
                 ImGuiWindowFlags_NoScrollbar);

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
    }

    drawSceneFn();
    // scene.on_render(renderer, assetManager);

    size_t texID = framebuffer.get_color_attachment_renderer_id();
    ImGui::Image((void*)texID, ImVec2{size.x, size.y}, ImVec2{0, 1},
                 ImVec2{1, 0});

    // get entity id at mouse position from attachement
    if (mousePosition.x >= 0 && mousePosition.y >= 0 &&
        mousePosition.x <= size.x && mousePosition.y <= size.y &&
        ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        // TODO getter for entity attachment index
        framebuffer.bind();
        m_HoveredEntity =
            framebuffer.read_pixel_data(1, {mousePosition.x, mousePosition.y})
                .UnsignedInt;
        framebuffer.unbind();

        if (m_HoveredEntity != (uint32_t)entt::null) {
            state.m_SelectedEntity = (entt::entity)m_HoveredEntity;
            auto& registry = scene.get_registry();
            auto identification = registry.get<core::components::Identifier>(
                state.m_SelectedEntity.value());
            ui::PropertiesPanel::add_tab(std::make_unique<EntityProperty>(
                registry, state.m_SelectedEntity.value(), identification.Name));
        }
    }

    // target for scene file drag and drop
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload =
                ImGui::AcceptDragDropPayload("CONTENT_BROWSER_SCENE_PATH")) {
            system::Path path = system::Path((const char*)payload->Data);

            changeSceneFn(path);
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

}  // namespace rr::editor::ui
