#include "rurouni/graphics/sprite.hpp"
#include "rurouni/graphics/texture.hpp"

namespace rr::graphics {

Sprite::Sprite(const SpriteSpecification& spec,
               std::weak_ptr<Spritesheet> spritesheet)
    : Texture(this, spec.Name, spec.Id),
      m_CellIndex(spec.CellIndex),
      m_CellSpread(spec.CellSpread),
      m_Spritesheet(spritesheet) {
    auto spritesheetPtr = spritesheet.lock();

    m_Size.x = spritesheetPtr->get_cell_size_px().x * m_CellSpread.x;
    m_Size.y = spritesheetPtr->get_cell_size_px().y * m_CellSpread.y;
    m_RendererId = spritesheetPtr->get_renderer_id();
    m_DistanceFieldType = spritesheetPtr->get_distance_field_type();

    // following units are all UV
    math::vec2 cellSize = spritesheetPtr->get_cell_size_uv();
    math::vec2 min;
    min.x = m_CellIndex.x * cellSize.x;
    min.y = m_CellIndex.y * cellSize.y;
    math::vec2 max;
    max.x = min.x + cellSize.x;
    max.y = min.y + cellSize.y;

    m_TextureCoords = {min, {max.x, min.y}, max, {min.x, max.y}};
}

}  // namespace rr::graphics
