#ifndef RR_LIB_GRAPHICS_SPRITE_H
#define RR_LIB_GRAPHICS_SPRITE_H

// rurouni
#include "rurouni/graphics/texture.hpp"
#include "rurouni/math/vec.hpp"

// std
#include <memory>

namespace rr::graphics {

class Sprite : public Texture {
   public:
    Sprite(const glm::ivec2& index,
           std::weak_ptr<Texture> spritesheet,
           const glm::ivec2& size_px,
           const UUID& uuid,
           const std::array<math::vec2, 4>& uvCoords);

    virtual std::optional<uint32_t> get_renderer_id() const override;

   private:
    math::ivec2 m_Index;
    std::weak_ptr<Texture> m_Spritesheet;
};

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_SPRITE_H
