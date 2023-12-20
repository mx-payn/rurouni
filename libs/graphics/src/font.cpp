// pch
#include <algorithm>
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/graphics/font.hpp"
#include "rurouni/graphics/logger.hpp"

// external
#include "cereal/archives/json.hpp"

namespace rr::graphics {

Font::Font(const ImageTextureSpecification& textureSpec,
           const FontMetrics& fontMetrics,
           const AtlasMetrics& atlasMetrics,
           const std::map<unicode_t, GlyphMetrics>& glyphMetrics)
    : Texture(textureSpec),
      m_FontMetrics(fontMetrics),
      m_AtlasMetrics(atlasMetrics),
      m_GlyphMetrics(glyphMetrics) {}

Font::Font(const FontSpecification& spec) : Texture(this, spec.TextureSpec) {
    // m_Shader = spec.FontShader;
    load_spec_from_file(spec.FontSpecPath);
    save_spec_to_file(spec.FontSpecPath.parent_path() / "test.json");
}

void Font::load_spec_from_file(const system::Path& path) {
    std::ifstream is(path);

    {
        std::vector<GlyphMetrics> glyphBuff;

        cereal::JSONInputArchive input(is);
        input(cereal::make_nvp("atlas", m_AtlasMetrics),
              cereal::make_nvp("metrics", m_FontMetrics),
              cereal::make_nvp("glyphs", glyphBuff)
              // cereal::make_nvp("kerning", m_KerningMetrics)
        );

        for (auto& glyph : glyphBuff) {
            m_GlyphMetrics[glyph.codepoint] = glyph;
        }
    }
    is.close();
}

void Font::save_spec_to_file(const system::Path& path) {
    std::ofstream os(path);
    {
        std::vector<GlyphMetrics> glyphBuff;
        glyphBuff.reserve(m_GlyphMetrics.size());
        for (auto& [key, glyph] : m_GlyphMetrics) {
            glyphBuff.push_back(glyph);
        }

        cereal::JSONOutputArchive out(os);
        out(cereal::make_nvp("atlas", m_AtlasMetrics),
            cereal::make_nvp("metrics", m_FontMetrics),
            cereal::make_nvp("glyphs", glyphBuff)
            // cereal::make_nvp("kerning", m_KerningMetrics)
        );
    }
    os.close();
}

const FontMetrics& Font::get_font_metrics() const {
    return m_FontMetrics;
}

const AtlasMetrics& Font::get_atlas_metrics() const {
    return m_AtlasMetrics;
}

const GlyphMetrics& Font::get_glyph_metrics(unicode_t codepoint) const {
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
