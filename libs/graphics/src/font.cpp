// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/dev/logger.hpp"
#include "rurouni/graphics/charsets.hpp"
#include "rurouni/graphics/font.hpp"
#include "rurouni/graphics/msdf_data.hpp"
#include "rurouni/math.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/types/uuid.hpp"

// external
#include <msdf-atlas-gen/msdf-atlas-gen.h>

namespace rr::graphics {

template <typename T,
          typename S,
          int N,
          msdf_atlas::GeneratorFunction<S, N> GenFunc>
static std::shared_ptr<Texture> CreateAndCacheAtlas(
    const std::string& fontName,
    double fontSize,
    const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
    const msdf_atlas::FontGeometry& fontGeometry,
    uint32_t width,
    uint32_t height) {
    msdf_atlas::GeneratorAttributes attributes;
    attributes.config.overlapSupport = true;
    attributes.scanlinePass = true;

    msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc,
                                        msdf_atlas::BitmapAtlasStorage<T, N>>
        generator(width, height);
    generator.setAttributes(attributes);
    generator.setThreadCount(8);
    generator.generate(glyphs.data(), (int)glyphs.size());

    msdfgen::BitmapConstRef<T, N> bitmap =
        (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

    TextureSpecification spec;
    spec.size = {bitmap.width, bitmap.height};
    spec.format = ImageFormat::RGB8;
    spec.generateMips = false;

    std::shared_ptr<Texture> texture = std::make_shared<Texture>(spec);
    texture->set_data((void*)bitmap.pixels, bitmap.width * bitmap.height * 3);
    return texture;
}

Font::Font(const system::Path& filepath,
           const UUID& uuid,
           double size /* = 40.0f */)
    : m_Filepath(filepath), m_UUID(uuid) {
    m_Data = new MSDFData();

    msdfgen::FreetypeHandle* freetype = msdfgen::initializeFreetype();
    dev::LOG->require(freetype, "could not initialize freetype");

    msdfgen::FontHandle* font = msdfgen::loadFont(freetype, filepath.c_str());
    dev::LOG->require(font, "could not load font file");

    msdf_atlas::Charset charset;
    for (uint32_t i = CHARSET_BASIC_LATIN[0]; i <= CHARSET_BASIC_LATIN[1];
         i++) {
        charset.add(i);
    }

    double fontScale = 1.0;
    m_Data->fontGeometry = msdf_atlas::FontGeometry(&m_Data->glyphs);
    int amountLoaded =
        m_Data->fontGeometry.loadCharset(font, fontScale, charset);
    dev::LOG->trace("loaded {} out of {} glyphs from font {} ", amountLoaded,
                    charset.size(), filepath.filename());

    double emSize = size;

    msdf_atlas::TightAtlasPacker atlasPacker;
    atlasPacker.setPixelRange(2.0);
    atlasPacker.setMiterLimit(1.0);
    atlasPacker.setPadding(0);
    atlasPacker.setScale(emSize);
    int remaining =
        atlasPacker.pack(m_Data->glyphs.data(), m_Data->glyphs.size());
    dev::LOG->require(remaining == 0,
                      "there were glyphs remaining after packing");

    int width, height;
    atlasPacker.getDimensions(width, height);
    dev::LOG->trace("packed atlas size({}, {})", width, height);

    const double DEFAULT_ANGLE_THRESHOLD = 3.0;
    const uint64_t LCG_MULTIPLIER = 6364136223846793005ull;
    const uint64_t LCG_INCREMENT = 1442695040888963407ull;
    const int THREAD_COUNT = 8;
    // if MSDF || MTSDF
    uint64_t coloringSeed = 0;
    bool expensiveColoring = false;
    if (expensiveColoring) {
        msdf_atlas::Workload(
            [&glyphs = m_Data->glyphs, &coloringSeed, DEFAULT_ANGLE_THRESHOLD](
                int i, int threadNo) -> bool {
                unsigned long long glyphSeed =
                    (LCG_MULTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT) *
                    !!coloringSeed;
                glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap,
                                       DEFAULT_ANGLE_THRESHOLD, glyphSeed);
                return true;
            },
            m_Data->glyphs.size())
            .finish(THREAD_COUNT);
    } else {
        unsigned long long glyphSeed = coloringSeed;
        for (msdf_atlas::GlyphGeometry& glyph : m_Data->glyphs) {
            glyphSeed *= LCG_MULTIPLIER;
            glyph.edgeColoring(msdfgen::edgeColoringInkTrap,
                               DEFAULT_ANGLE_THRESHOLD, glyphSeed);
        }
    }

    m_Texture =
        CreateAndCacheAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>(
            "test", emSize, m_Data->glyphs, m_Data->fontGeometry, width,
            height);

    msdfgen::destroyFont(font);
    msdfgen::deinitializeFreetype(freetype);

    for (auto& glyph : m_Data->glyphs) {
        double al, ab, ar, at;
        glyph.getQuadAtlasBounds(al, ab, ar, at);
        math::vec2 texCoordMin((float)al, (float)ab);
        math::vec2 texCoordMax((float)ar, (float)at);

        float texelWidth = 1.0f / m_Texture->get_size().x;
        float texelHeight = 1.0f / m_Texture->get_size().y;
        texCoordMin *= math::vec2(texelWidth, texelHeight);
        texCoordMax *= math::vec2(texelWidth, texelHeight);

        // TODO codepoint is of type unicode_t which is uint32_t
        m_CharSubtexturePositions[(char)glyph.getCodepoint()] = {
            math::vec2{texCoordMin.x, texCoordMin.y},
            math::vec2{texCoordMin.x, texCoordMax.y},
            math::vec2{texCoordMax.x, texCoordMax.y},
            math::vec2{texCoordMax.x, texCoordMin.y}};
    }
}

Font::~Font() {}

}  // namespace rr::graphics
