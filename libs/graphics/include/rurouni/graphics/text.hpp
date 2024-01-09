#ifndef RR_LIB_GRAPHICS_TEXT_H
#define RR_LIB_GRAPHICS_TEXT_H

#include "rurouni/graphics/font.hpp"
#include "rurouni/graphics/texture.hpp"
#include "rurouni/math.hpp"

#include <cstdint>
#include <memory>

namespace rr::graphics {

class Text : public Texture {
   public:
    Text(const std::u32string& text,
         std::weak_ptr<Font> font,
         math::ivec2 lineMetrics_px);
    ~Text() { m_RendererId = 0; }

    void update(float dt);

   private:
    std::u32string m_Text;

    std::weak_ptr<Font> m_Font;
};

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_TEXT_H
