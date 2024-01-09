#ifndef RR_LIBS_GRAPHICS_SPRITESHEET_H
#define RR_LIBS_GRAPHICS_SPRITESHEET_H

#include <memory>
#include "rurouni/graphics/texture.hpp"
#include "rurouni/math/vec.hpp"

namespace rr::graphics {

struct SpritesheetSpecification {
    std::string Name = "unnamed";
    UUID Id = UUID();
    UUID TextureId = UUID();

    math::ivec2 CellCount = math::ivec2(1.0f);
    std::array<math::vec2, 4> TextureCoords = {
        math::vec2{0.0f, 0.0f}, math::vec2{1.0f, 0.0f}, math::vec2{1.0f, 1.0f},
        math::vec2{0.0f, 1.0f}};
};

class Spritesheet : public Texture {
   public:
    Spritesheet(const SpritesheetSpecification& spec,
                std::weak_ptr<Texture> texture)
        : Texture(this, spec.Name, spec.Id),
          m_CellCount(spec.CellCount),
          m_Texture(texture) {
        auto texturePtr = texture.lock();

        // init sheet (this) members
        m_CellSize_PX.x = texturePtr->get_size().x / m_CellCount.x;
        m_CellSize_PX.y = texturePtr->get_size().y / m_CellCount.y;

        m_CellSize_UV.x = 1.0f / m_CellCount.x;
        m_CellSize_UV.y = 1.0f / m_CellCount.y;

        // init texture members
        m_TextureCoords = spec.TextureCoords;
        math::ivec2 textureSize_PX = texturePtr->get_size();
        math::vec2 sheetSize_UV =
            math::vec2(m_TextureCoords[2].x - m_TextureCoords[0].x,
                       m_TextureCoords[2].y - m_TextureCoords[0].y);
        m_Size.x = textureSize_PX.x * sheetSize_UV.x;
        m_Size.y = textureSize_PX.y * sheetSize_UV.y;
        m_RendererId = texturePtr->get_renderer_id();
        m_DistanceFieldType = texturePtr->get_distance_field_type();
    }

    ~Spritesheet() { m_RendererId = 0; }

    const math::ivec2& get_cell_count() const { return m_CellCount; }
    const math::ivec2& get_cell_size_px() const { return m_CellSize_PX; }
    const math::vec2& get_cell_size_uv() const { return m_CellSize_UV; }

   private:
    math::ivec2 m_CellCount;
    math::ivec2 m_CellSize_PX;
    math::vec2 m_CellSize_UV;

    std::weak_ptr<Texture> m_Texture;
};

}  // namespace rr::graphics

#endif  // !RR_LIBS_GRAPHICS_SPRITESHEET_H
