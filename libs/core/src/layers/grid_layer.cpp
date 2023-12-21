#include "rurouni/core/components/orthographic_projection.hpp"
#include "rurouni/core/components/transform.hpp"
#include "rurouni/pch.hpp"

#include "rurouni/core/layers/grid_layer.hpp"

#include <cmath>

namespace rr::core {

GridLayer::GridLayer() : Layer("grid_layer") {}

GridLayer::~GridLayer() {}

void GridLayer::on_render(graphics::BatchRenderer& renderer,
                          const entt::registry& registry,
                          const SceneState& sceneState) {
    auto components =
        registry
            .try_get<components::Transform, components::OrthographicProjection>(
                sceneState.LayerCameraId);

    if (std::get<0>(components) && std::get<1>(components)) {
        auto transformPtr = std::get<0>(components);
        auto projectionPtr = std::get<1>(components);

        float startX =
            transformPtr->get_translation().x + projectionPtr->get_left();
        float startY =
            transformPtr->get_translation().y + projectionPtr->get_top();

        float endX =
            transformPtr->get_translation().x + projectionPtr->get_right();
        float endY =
            transformPtr->get_translation().y + projectionPtr->get_bottom();

        float f;
        float offsetX = std::modf(endX, &f);
        float offsetY = std::modf(endY, &f);

        math::mat4 gridCellTransform;
        for (float y = startY + offsetY; y < endY; y++) {
            for (float x = startX + offsetX; x < endX; x++) {
                gridCellTransform =
                    math::translate(math::mat4(1.0f), math::vec3(x, y, 0.0f));
                renderer.draw_rect(gridCellTransform, m_GridColor,
                                   (uint32_t)entt::null);
            }
        }
    }
}

}  // namespace rr::core
