#ifndef RR_LIBS_CORE_SCENE_H
#define RR_LIBS_CORE_SCENE_H

// rurouni
#include "entt/entity/fwd.hpp"
#include "rurouni/core/layer.hpp"
#include "rurouni/core/scene_state.hpp"
#include "rurouni/error.hpp"
#include "rurouni/graphics/batch_renderer.hpp"
#include "rurouni/graphics/framebuffer.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/types/uuid.hpp"

// external
#include <entt/entt.hpp>

// std
#include <memory>
#include <vector>

namespace rr::core {

class Scene {
   public:
    Scene(const math::ivec2& viewportSize_px);
    ~Scene();

    std::optional<Error> read_from_file(const system::Path& filepath,
                                        const math::ivec2& viewportSize_px);
    std::optional<Error> write_to_file(const system::Path& path);

    void on_update(float dt);
    void on_render(graphics::BatchRenderer& renderer);

    void push_layer(std::unique_ptr<Layer> layer);
    void pop_layer();
    void push_overlay(std::unique_ptr<Layer> overlay);
    void pop_overlay();
    void set_debug_layer(std::shared_ptr<Layer> layer);

    void set_viewport_size(const math::ivec2& size);

    graphics::Framebuffer& get_framebuffer() const { return *m_Framebuffer; }
    entt::registry& get_registry() { return m_Registry; }

   private:
    void update_camera_data();

   private:
    std::string m_Name;
    system::Path m_Filepath;

    std::unique_ptr<graphics::Framebuffer> m_Framebuffer;

    // layers
    std::vector<std::unique_ptr<Layer>> m_Layers;
    std::vector<std::unique_ptr<Layer>> m_Overlays;
    std::shared_ptr<Layer> m_DebugLayer;

    // scene state
    SceneState m_SceneState;
    entt::registry m_Registry;
};

}  // namespace rr::core

#endif  // !RR_LIBS_CORE_SCENE_H
