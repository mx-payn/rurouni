#ifndef RR_LIB_GRAPHICS_FONT_H
#define RR_LIB_GRAPHICS_FONT_H

// rurouni
#include "rurouni/graphics/logger.hpp"
#include "rurouni/graphics/shader.hpp"
#include "rurouni/graphics/texture.hpp"
#include "rurouni/math.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/types/uuid.hpp"

// external
#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <magic_enum.hpp>

// std
#include <array>
#include <map>
#include <memory>
#include <optional>

namespace rr::graphics {

enum class FontAtlasType { None, HardMask, SoftMask, SDF, PSDF, MSDF, MTSDF };
enum class YOrigin { None, Bottom, Top };

typedef uint32_t unicode_t;

struct FontMetrics {
    /// The size of one EM.
    double emSize;
    /// The vertical position of the ascender and descender relative to the
    /// baseline.
    double ascender, descender;
    /// The vertical difference between consecutive baselines.
    double lineHeight;
    /// The vertical position and thickness of the underline.
    double underlineY, underlineThickness;
};

/** units in pixel */
struct AtlasMetrics {
    FontAtlasType type;             // "msdf",
    float distanceRange;            // "distanceRange": 2,
    float size;                     //    "size": 40,
    int32_t width;                  //    "width": 256,
    int32_t height;                 // "height": 256,
    rr::graphics::YOrigin yOrigin;  //    "yOrigin": "bottom"
};

struct Rect {
    float left, bottom, right, top;
};

struct GlyphMetrics {
    unicode_t codepoint;
    double advance;
    /** uv texture position */
    Rect planeBounds;
    /** pixel texture position */
    Rect atlasBounds;
};

struct GlyphRenderSpecification {
    unicode_t codepoint;
    std::array<math::vec2, 2> TextureBounds_UV;
    // TODO maybe not needed
    std::array<math::vec2, 2> TextureBounds_PX;
};

struct FontSpecification {
    ImageTextureSpecification TextureSpec;
    system::Path FontSpecPath;
    std::optional<std::weak_ptr<Shader>> FontShader;
};

class Font : public Texture {
   public:
    Font(const FontSpecification& spec);
    ~Font() = default;

    const FontMetrics& get_font_metrics() const;
    const AtlasMetrics& get_atlas_metrics() const;
    const GlyphMetrics& get_glyph_metrics(unicode_t codepoint) const;
    float get_kerning_distance(unicode_t left, unicode_t right);

   private:
    void load_spec_from_file(const system::Path& path);
    void save_spec_to_file(const system::Path& path);

   protected:
    FontMetrics m_FontMetrics;
    AtlasMetrics m_AtlasMetrics;

    std::map<unicode_t, GlyphMetrics> m_GlyphMetrics;
    // std::multimap<unicode_t, std::pair<unicode_t, float>> m_KerningMetrics;
};

template <typename Archive>
void serialize(Archive& archive, rr::graphics::FontMetrics& metrics) {
    archive(cereal::make_nvp("emSize", metrics.emSize),
            cereal::make_nvp("ascender", metrics.ascender),
            cereal::make_nvp("descender", metrics.descender),
            cereal::make_nvp("lineHeight", metrics.lineHeight),
            cereal::make_nvp("underlineY", metrics.underlineY),
            cereal::make_nvp("underlineThickness", metrics.underlineThickness));
}

template <typename Archive>
void serialize(Archive& archive, rr::graphics::AtlasMetrics& metrics) {
    archive(cereal::make_nvp("type", metrics.type),
            cereal::make_nvp("distanceRange", metrics.distanceRange),
            cereal::make_nvp("size", metrics.size),
            cereal::make_nvp("width", metrics.width),
            cereal::make_nvp("height", metrics.height),
            cereal::make_nvp("yOrigin", metrics.yOrigin));
}

template <typename Archive>
void serialize(Archive& archive, rr::graphics::Rect& rect) {
    archive(cereal::make_nvp("left", rect.left),
            cereal::make_nvp("bottom", rect.bottom),
            cereal::make_nvp("right", rect.right),
            cereal::make_nvp("top", rect.top));
}

template <typename Archive>
void serialize(Archive& archive, rr::graphics::GlyphMetrics& metrics) {
    try {
        archive(cereal::make_nvp("unicode", metrics.codepoint),
                cereal::make_nvp("advance", metrics.advance),
                // these will throw if not present (e.g. space character)
                // don't know yet how to properly handle not present values
                cereal::make_nvp("planeBounds", metrics.planeBounds),
                cereal::make_nvp("atlasBounds", metrics.atlasBounds));
    } catch (cereal::Exception e) {
        warn("cereal exception at character '{}': {}", metrics.codepoint,
             e.what());
    }
}

template <class Archive>
std::string save_minimal(Archive const&, FontAtlasType const& obj) {
    auto name = magic_enum::enum_name(obj);
    std::string nameStr = std::string{name};

    std::transform(nameStr.begin(), nameStr.end(), nameStr.begin(),
                   [](unsigned char c) { return std::tolower(c); }  // correct
    );

    return nameStr;
}

template <class Archive>
void load_minimal(Archive const&,
                  FontAtlasType& obj,
                  std::string const& value) {
    auto type = magic_enum::enum_cast<FontAtlasType>(
        value, magic_enum::case_insensitive);
    if (type.has_value())
        obj = type.value();
    else
        obj = FontAtlasType::None;
}

template <class Archive>
std::string save_minimal(Archive const&, YOrigin const& obj) {
    auto name = magic_enum::enum_name(obj);
    std::string nameStr = std::string{name};

    std::transform(nameStr.begin(), nameStr.end(), nameStr.begin(),
                   [](unsigned char c) { return std::tolower(c); }  // correct
    );

    return nameStr;
}

template <class Archive>
void load_minimal(Archive const&, YOrigin& obj, std::string const& value) {
    auto type =
        magic_enum::enum_cast<YOrigin>(value, magic_enum::case_insensitive);
    if (type.has_value())
        obj = type.value();
    else
        obj = YOrigin::None;
}

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_FONT_H
