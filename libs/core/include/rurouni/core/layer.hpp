#ifndef RR_LIBS_CORE_LAYER_H
#define RR_LIBS_CORE_LAYER_H

// rurouni
#include "entt/entity/fwd.hpp"
#include "rurouni/core/grid_state.hpp"
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
                           entt::registry& registry,
                           const GridState& gridState) = 0;
};

}  // namespace rr::core

#endif  // !RR_LIBS_CORE_LAYER_H
