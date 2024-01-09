#ifndef RR_LIB_GRAPHICS_SPRITE_H
#define RR_LIB_GRAPHICS_SPRITE_H

// rurouni
#include "rurouni/graphics/spritesheet.hpp"
#include "rurouni/graphics/texture.hpp"
#include "rurouni/math/vec.hpp"

// std
#include <memory>

namespace rr::graphics {

struct SpriteSpecification {
    std::string Name = "unnamed";
    UUID Id = UUID();
    UUID SpritesheetId = UUID();

    math::ivec2 CellIndex = math::ivec2(0.0f);
    math::ivec2 CellSpread = math::ivec2(1.0f);
};

class Sprite : public Texture {
   public:
    Sprite(const SpriteSpecification& spec,
           std::weak_ptr<Spritesheet> spritesheet);
    ~Sprite() { m_RendererId = 0; }

    const math::ivec2& get_cell_index() const { return m_CellIndex; }
    const math::ivec2& get_cell_spread() const { return m_CellSpread; }

   private:
    math::ivec2 m_CellIndex;
    math::ivec2 m_CellSpread;
    std::weak_ptr<Spritesheet> m_Spritesheet;
};

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_SPRITE_H
