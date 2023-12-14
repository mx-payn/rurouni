#include "rurouni/pch.hpp"

#include "rurouni/core/layers/grid_layer.hpp"

namespace rr::core {

GridLayer::GridLayer(const GridState& gridState) : Layer() {
    graphics::DataTextureSpecification gridSpec;
    gridSpec.dataFormat = graphics::TextureDataFormat::RGBA;
    gridSpec.pixelFormat = graphics::TexturePixelFormat::RGBA8;
    gridSpec.size = gridState.CellSize_px;
    m_GridTexture = std::make_unique<graphics::Texture>(gridSpec);

    std::vector<uint32_t> gridCellData;
    for (int y = 0; y < gridSpec.size.y; y++) {
        for (int x = 0; x < gridSpec.size.x; x++) {
            if (y == 0 || y == gridSpec.size.y - 1 || x == 0 ||
                x == gridSpec.size.x - 1) {
                gridCellData.push_back(0xFFFFFFFF);
            } else {
                gridCellData.push_back(0x00000000);
            }
        }
    }
    m_GridTexture->set_data(gridCellData.data(),
                            gridCellData.size() * sizeof(uint32_t));
}

GridLayer::~GridLayer() {}

void GridLayer::on_render(graphics::BatchRenderer& renderer,
                          const GridState& gridState) {
    // draw grid
    math::mat4 gridCellTransform;
    for (int y = 0; y < gridState.CellCount.y; y++) {
        for (int x = 0; x < gridState.CellCount.x; x++) {
            gridCellTransform =
                math::translate(math::mat4(1.0f), math::vec3(x, y, 0.0f));
            renderer.draw_texture(gridCellTransform, m_GridTexture,
                                  {0.2, 0.2, 0.2, 1.0f}, 0);
        }
    }
}

}  // namespace rr::core
