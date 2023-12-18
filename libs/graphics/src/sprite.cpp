#include "rurouni/graphics/sprite.hpp"
#include "rurouni/graphics/texture.hpp"

namespace rr::graphics {

Sprite::Sprite(const glm::ivec2& index,
               std::weak_ptr<Texture> spritesheet,
               const glm::ivec2& size_px,
               const UUID& uuid,
               const std::array<math::vec2, 4>& uvCoords)
    : Texture(this), m_Index(index), m_Spritesheet(spritesheet) {
    m_Size = size_px;
    m_UUID = uuid;
    m_UVCoords = uvCoords;
}

std::optional<uint32_t> Sprite::get_renderer_id() const {
    if (m_Spritesheet.expired()) {
        return {};
    } else {
        return m_Spritesheet.lock()->get_renderer_id();
    }
}

}  // namespace rr::graphics
