#ifndef RR_LIBS_CORE_LAYER_H
#define RR_LIBS_CORE_LAYER_H

// rurouni
#include "entt/entity/fwd.hpp"
#include "rurouni/core/scene_state.hpp"
#include "rurouni/graphics/batch_renderer.hpp"

// external
#include <entt/entt.hpp>

namespace rr::core {

class Layer {
   public:
    Layer() {}
    virtual ~Layer() {}

    virtual void on_attach() {}
    virtual void on_detach() {}

    virtual void on_render(graphics::BatchRenderer& renderer,
                           const entt::registry& registry,
                           const SceneState& sceneState) = 0;

   private:
    friend class cereal::access;
    template <typename Archive>
    void serialize(Archive& archive) {}
};

}  // namespace rr::core

#endif  // !RR_LIBS_CORE_LAYER_H
