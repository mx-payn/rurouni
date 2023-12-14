#ifndef RR_LIBS_CORE_LAYER_H
#define RR_LIBS_CORE_LAYER_H

#include "rurouni/core/grid_state.hpp"
#include "rurouni/graphics/batch_renderer.hpp"

namespace rr::core {

class Layer {
   public:
    Layer() {}
    virtual ~Layer() {}

    virtual void on_attach() {}
    virtual void on_detach() {}

    virtual void on_render(graphics::BatchRenderer& renderer,
                           const GridState& gridState) = 0;
};

}  // namespace rr::core

#endif  // !RR_LIBS_CORE_LAYER_H
