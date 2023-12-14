#ifndef RR_LIBS_CORE_LAYERS_GRID_LAYER_H
#define RR_LIBS_CORE_LAYERS_GRID_LAYER_H

#include "rurouni/core/grid_state.hpp"
#include "rurouni/core/layer.hpp"
#include "rurouni/graphics/texture.hpp"
#include "rurouni/math/vec.hpp"

#include <memory>

namespace rr::core {

class GridLayer : public Layer {
   public:
    GridLayer(const GridState& gridState);
    ~GridLayer();

    virtual void on_render(graphics::BatchRenderer& renderer,
                           const GridState& gridState) override;

   private:
    std::shared_ptr<graphics::Texture> m_GridTexture;
};

}  // namespace rr::core

#endif  // !RR_LIBS_CORE_LAYERS_GRID_LAYER_H
