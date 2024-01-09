// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/graphics/font.hpp"
#include "rurouni/graphics/logger.hpp"
#include "rurouni/graphics/text.hpp"
#include "rurouni/graphics/texture.hpp"
#include "rurouni/math.hpp"

namespace rr::graphics {

// TODO: fix this class
Text::Text(const std::u32string& text,
           std::weak_ptr<Font> font,
           math::ivec2 textBounds_px)
    : Texture(this, "", UUID()), m_Text(text), m_Font(font) {
    if (text.empty())
        return;

    std::vector<GlyphRenderSpecification> renderSpecs;
    auto fontPtr = font.lock();

    /////////////////////////////////////////////
    /// data and variables
    const auto& fontMetrics = fontPtr->get_font_metrics();
    const auto& atlasMetrics = fontPtr->get_atlas_metrics();
    math::vec2 texelSize = {1.0f / atlasMetrics.width,
                            1.0f / atlasMetrics.height};

    // variables relative to box size
    float lineHeight = textBounds_px.y;
    float fsScale =
        (1.0f * lineHeight) / (fontMetrics.ascender - fontMetrics.descender);

    float textStartY = static_cast<float>(1080.0f) - (fontMetrics.lineHeight);
    math::vec2 currentPosition = math::vec2(0.0f, 0.0f);

    /////////////////////////////////////////////
    ///  example character
    unicode_t character = 'M';
    const auto& glyphMetrics = fontPtr->get_glyph_metrics(character);

    Rect quadAtlasBounds = glyphMetrics.atlasBounds;
    math::vec2 texCoordMin(quadAtlasBounds.left, quadAtlasBounds.bottom);
    math::vec2 texCoordMax(quadAtlasBounds.right, quadAtlasBounds.top);
    texCoordMin *= texelSize;
    texCoordMax *= texelSize;

    Rect quadPlaneBounds = glyphMetrics.planeBounds;
    math::vec2 quadMin(quadPlaneBounds.left, quadPlaneBounds.bottom);
    math::vec2 quadMax(quadPlaneBounds.right, quadPlaneBounds.top);

    quadMin *= fsScale;
    quadMax *= fsScale;
    quadMin *= math::vec2(1920, 1080);
    quadMax *= math::vec2(1920, 1080);
    quadMin += currentPosition;
    quadMax += currentPosition;

    // TODO add kerning
    currentPosition.x += fsScale * glyphMetrics.advance;  // + kerningOffset

    // glm::vec2 genQuad = {(quadMax.x - quadMin.x) / texelSize.x, (quadMax.y -
    // quadMin.y) / texelSize.y}; glm::vec2 drawQuad = {(currentPosition.x *
    // fsScale) / texelSize.x, (fontMetrics.lineHeight * fsScale) /
    // texelSize.y};

    /////////////////////////////////////////////
    ///  create render glyphs
    ///  TODO
}

void Text::update(float dt) {}

}  // namespace rr::graphics
