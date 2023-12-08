#ifndef RR_LIB_GRAPHICS_TEXT_H
#define RR_LIB_GRAPHICS_TEXT_H

#include "rurouni/graphics/font.hpp"
#include "rurouni/graphics/texture.hpp"
#include "rurouni/math.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace rr::graphics {

struct Character {
    uint32_t Codepoint;
    std::array<math::vec2, 4> textureCoords_uv;
    std::array<math::vec2, 4> quadPosition_uv;

    Character(uint32_t codepoint,
              std::array<math::vec2, 4> texCoords_uv,
              std::array<math::vec2, 4> quadPos_uv)
        : Codepoint(codepoint),
          textureCoords_uv(texCoords_uv),
          quadPosition_uv(quadPos_uv) {}
};

class Text {
   public:
    Text() = default;

    const std::vector<Character>& get_characters() const {
        return m_Characters;
    }
    std::weak_ptr<Texture> get_font_atlas() const { return m_FontAtlas; }
    std::weak_ptr<Font> get_font() const { return m_Font; }

    const math::vec2& get_size() const { return m_Size; }
    const double get_line_height() const { return m_LineHeight; }

   protected:
    std::vector<Character> m_Characters;

    math::vec2 m_Size;
    double m_LineHeight;
    std::weak_ptr<Texture> m_FontAtlas;
    std::weak_ptr<Font> m_Font;
};

class WrappedText : public Text {
   public:
    WrappedText(const std::string& text,
                std::weak_ptr<Font> font,
                float maxWidth,
                float lineHeight,
                float horizontalPadding);
};

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_TEXT_H
