#ifndef RR_LIBS_CORE_SCENE_H
#define RR_LIBS_CORE_SCENE_H

#include "rurouni/core/grid_state.hpp"
#include "rurouni/core/layer.hpp"
#include "rurouni/graphics/batch_renderer.hpp"
#include "rurouni/graphics/framebuffer.hpp"

#include <memory>
#include <vector>

namespace rr::core {

class Scene {
   public:
    Scene(const math::ivec2& viewportSize_px);
    ~Scene();

    void on_update(float dt);
    void on_render(graphics::BatchRenderer& renderer);

    void push_layer(std::unique_ptr<Layer> layer);
    void pop_layer();
    void push_overlay(std::unique_ptr<Layer> overlay);
    void pop_overlay();
    void set_debug_layer(std::shared_ptr<Layer> layer);

    void set_viewport_size(const math::ivec2& size);

    graphics::Framebuffer& get_framebuffer() const { return *m_Framebuffer; }

   private:
    math::ivec2 m_ViewportSize_px;
    std::unique_ptr<graphics::Framebuffer> m_Framebuffer;

    // layers
    std::vector<std::unique_ptr<Layer>> m_Layers;
    std::vector<std::unique_ptr<Layer>> m_Overlays;
    std::shared_ptr<Layer> m_DebugLayer;

    // cameras
    math::mat4 m_CameraTransform;
    math::mat4 m_CameraProjection;

    // scene state
    GridState m_GridState;
};
}  // namespace rr::core

#endif  // !RR_LIBS_CORE_SCENE_H
