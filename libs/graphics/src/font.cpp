// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/graphics/font.hpp"
#include "rurouni/graphics/logger.hpp"

namespace rr::graphics {

Font::Font(const FontSpecification& spec, std::weak_ptr<Texture> atlas)
    : Texture(this, spec.Name, spec.Id),
      m_AtlasMetrics(spec.AtlasMetrics),
      m_FontMetrics(spec.FontMetrics),
      m_GlyphMetrics(spec.GlyphMetrics),
      m_FontAtlas(atlas) {
    auto atlasPtr = atlas.lock();
    m_Size = atlasPtr->get_size();
    m_RendererId = atlasPtr->get_renderer_id();
    m_DistanceFieldType = atlasPtr->get_distance_field_type();
}

const FontMetrics& Font::get_font_metrics() {
    return m_FontMetrics;
}

const AtlasMetrics& Font::get_atlas_metrics() const {
    return m_AtlasMetrics;
}

const GlyphMetrics& Font::get_glyph_metrics(unicode_t codepoint) {
    if (m_GlyphMetrics.find(codepoint) == m_GlyphMetrics.end()) {
        return m_GlyphMetrics.at('?');
    }

    return m_GlyphMetrics.at(codepoint);
}

float Font::get_kerning_distance(unicode_t left, unicode_t right) {
    // TODO implement kerning
    return 0.0f;
}

}  // namespace rr::graphics
