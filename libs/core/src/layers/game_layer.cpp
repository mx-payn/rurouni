// pch
#include "rurouni/pch.hpp"
//-----------------------
// rurouni
#include "rurouni/core/components.hpp"
#include "rurouni/core/layers/game_layer.hpp"

namespace rr::core {

GameLayer::GameLayer() : Layer("game_layer") {}

GameLayer::~GameLayer() {}

void GameLayer::on_render(graphics::BatchRenderer& renderer,
                          AssetManager& assetManager,
                          entt::registry& registry,
                          const SceneState& sceneState) {
    auto textureView =
        registry.view<components::Transform, components::Texture>();
    for (auto&& [entity, transform, texture] : textureView.each()) {
        if (texture.TextureUUID.is_null())
            continue;

        renderer.draw_texture(transform.get_transform(),
                              assetManager.get_texture(texture.TextureUUID),
                              static_cast<uint32_t>(entity));
    }
}

}  // namespace rr::core
