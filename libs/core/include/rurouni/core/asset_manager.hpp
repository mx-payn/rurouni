#ifndef RR_LIBS_CORE_ASSET_MANAGER_H
#define RR_LIBS_CORE_ASSET_MANAGER_H

#include "magic_enum.hpp"
#include "rurouni/error.hpp"
#include "rurouni/graphics/font.hpp"
#include "rurouni/graphics/shader.hpp"
#include "rurouni/graphics/sprite.hpp"
#include "rurouni/graphics/spritesheet.hpp"
#include "rurouni/graphics/texture.hpp"
#include "rurouni/math.hpp"
#include "rurouni/math/vec.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/types/uuid.hpp"

#include <array>
#include <map>
#include <memory>
#include <optional>
#include <unordered_map>

#include "cereal/cereal.hpp"
#include "cereal/types/array.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/optional.hpp"
#include "cereal/types/vector.hpp"

namespace rr::core {

class AssetManager {
   public:
    AssetManager(const system::Path& rootDir);
    ~AssetManager();

    std::optional<Error> read_asset_configuration();
    std::optional<Error> write_asset_configuration();

    void register_image_texture(
        const graphics::ImageTextureSpecification& spec);
    void register_spritesheet(const graphics::SpritesheetSpecification& spec);
    void register_sprite(const graphics::SpriteSpecification& spec);
    void register_font(const graphics::FontSpecification& spec);
    void register_shader(const graphics::ShaderSpecification spec);

    void load_image_texture(const UUID& uuid);
    void load_spritesheet(const UUID& uuid);
    void load_sprite(const UUID& uuid);
    void load_font(const UUID& uuid);
    void load_shader(const UUID& uuid);

    void unload_texture(const UUID& uuid);
    void unload_shader(const UUID& uuid);

    std::weak_ptr<graphics::Texture> get_texture(const UUID& uuid);
    std::weak_ptr<graphics::Shader> get_shader(const UUID& uuid);

    std::unordered_map<UUID, graphics::ImageTextureSpecification>&
    get_image_texture_database() {
        return m_ImageTextureDatabase;
    }
    std::unordered_map<UUID, graphics::SpritesheetSpecification>&
    get_spritesheet_database() {
        return m_SpritesheetDatabase;
    }
    std::unordered_map<UUID, graphics::SpriteSpecification>&
    get_sprite_database() {
        return m_SpriteDatabase;
    }
    std::unordered_map<UUID, graphics::FontSpecification>& get_font_database() {
        return m_FontDatabase;
    }
    std::unordered_map<UUID, graphics::ShaderSpecification>&
    get_shader_database() {
        return m_ShaderDatabase;
    }

    bool has_image_texture_specification(const UUID& uuid);
    bool texture_is_loaded(const UUID& uuid);

   private:
    system::Path m_RootDir;

    std::unordered_map<UUID, graphics::ImageTextureSpecification>
        m_ImageTextureDatabase;
    std::unordered_map<UUID, graphics::SpritesheetSpecification>
        m_SpritesheetDatabase;
    std::unordered_map<UUID, graphics::SpriteSpecification> m_SpriteDatabase;
    std::unordered_map<UUID, graphics::FontSpecification> m_FontDatabase;
    // std::unordered_map<UUID, graphics::TextSpecification> m_TextDatabase;
    std::unordered_map<UUID, graphics::ShaderSpecification> m_ShaderDatabase;

    std::unordered_map<UUID, std::shared_ptr<graphics::Texture>>
        m_LoadedTextureCache;
    std::unordered_map<UUID, std::shared_ptr<graphics::Shader>>
        m_LoadedShaderCache;

    const UUID DEFAULT_TEXTURE_ID = UUID::create();
    const UUID DEFAULT_SPRITESHEET_ID = UUID::create();
    const UUID DEFAULT_SPRITE_ID = UUID::create();
    const UUID DEFAULT_FONT_ID = UUID::create();
    const UUID DEFAULT_SHADER_ID = UUID::create();
};

}  // namespace rr::core

#include "rurouni/base64.h"
namespace rr::graphics {

template <typename Archive>
void serialize(Archive& archive,
               rr::graphics::ImageTextureSpecification& spec) {
    archive(cereal::make_nvp("name", spec.Name),
            cereal::make_nvp("id", spec.Id),
            cereal::make_nvp("filepath", spec.Filepath),
            cereal::make_nvp("distance_field_type", spec.DistanceFieldType));
}

template <class Archive>
std::string save_minimal(Archive const&,
                         rr::graphics::TextureDataFormat const& format) {
    std::string formatStr = std::string(magic_enum::enum_name(format));
    std::transform(formatStr.begin(), formatStr.end(), formatStr.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return std::string(formatStr);
}

template <class Archive>
void load_minimal(Archive const&,
                  rr::graphics::TextureDataFormat& format,
                  std::string const& value) {
    auto type = magic_enum::enum_cast<rr::graphics::TextureDataFormat>(
        value, magic_enum::case_insensitive);
    if (type.has_value())
        format = type.value();
    else
        format = rr::graphics::TextureDataFormat::None;
}

template <class Archive>
std::string save_minimal(Archive const&,
                         rr::graphics::TexturePixelFormat const& format) {
    std::string formatStr = std::string(magic_enum::enum_name(format));
    std::transform(formatStr.begin(), formatStr.end(), formatStr.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return std::string(formatStr);
}

template <class Archive>
void load_minimal(Archive const&,
                  rr::graphics::TexturePixelFormat& format,
                  std::string const& value) {
    auto type = magic_enum::enum_cast<rr::graphics::TexturePixelFormat>(
        value, magic_enum::case_insensitive);
    if (type.has_value())
        format = type.value();
    else
        format = rr::graphics::TexturePixelFormat::None;
}

template <class Archive>
void save(Archive& archive,
          rr::graphics::DataTextureSpecification const& spec) {
    archive(
        cereal::make_nvp("name", spec.Name), cereal::make_nvp("id", spec.Id),
        cereal::make_nvp("size", spec.Size),
        cereal::make_nvp("data_format", spec.DataFormat),
        cereal::make_nvp("pixel_format", spec.PixelFormat),
        cereal::make_nvp("generate_mips", spec.GenerateMips),
        cereal::make_nvp("distance_field_type", spec.DistanceFieldType),
        cereal::make_nvp(
            "data", rr::base64_encode(spec.get_data(), spec.get_data_size())),
        cereal::make_nvp("data_size", spec.get_data_size()));
}

template <class Archive>
void load(Archive& archive, rr::graphics::DataTextureSpecification& spec) {
    std::vector<uint8_t> data;
    size_t dataSize;
    archive(cereal::make_nvp("name", spec.Name),
            cereal::make_nvp("id", spec.Id),
            cereal::make_nvp("size", spec.Size),
            cereal::make_nvp("data_format", spec.DataFormat),
            cereal::make_nvp("pixel_format", spec.PixelFormat),
            cereal::make_nvp("generate_mips", spec.GenerateMips),
            cereal::make_nvp("distance_field_type", spec.DistanceFieldType),
            cereal::make_nvp("data", data),
            cereal::make_nvp("data_size", dataSize));

    spec.set_data(data.data(), dataSize);
}

template <typename Archive>
void serialize(Archive& archive, rr::graphics::SpritesheetSpecification& spec) {
    archive(cereal::make_nvp("name", spec.Name),
            cereal::make_nvp("id", spec.Id),
            cereal::make_nvp("texture_id", spec.TextureId),
            cereal::make_nvp("cell_count", spec.CellCount),
            cereal::make_nvp("texture_coords", spec.TextureCoords));
}

template <typename Archive>
void serialize(Archive& archive, rr::graphics::SpriteSpecification& spec) {
    archive(cereal::make_nvp("name", spec.Name),
            cereal::make_nvp("id", spec.Id),
            cereal::make_nvp("spritesheet_id", spec.SpritesheetId),
            cereal::make_nvp("cell_index", spec.CellIndex),
            cereal::make_nvp("cell_spread", spec.CellSpread));
}

template <class Archive>
std::string save_minimal(Archive const&, rr::graphics::YOrigin const& obj) {
    auto name = magic_enum::enum_name(obj);
    std::string nameStr = std::string{name};

    std::transform(nameStr.begin(), nameStr.end(), nameStr.begin(),
                   [](unsigned char c) { return std::tolower(c); }  // correct
    );

    return nameStr;
}

template <class Archive>
void load_minimal(Archive const&,
                  rr::graphics::YOrigin& obj,
                  std::string const& value) {
    auto type =
        magic_enum::enum_cast<YOrigin>(value, magic_enum::case_insensitive);
    if (type.has_value())
        obj = type.value();
    else
        obj = YOrigin::None;
}

template <typename Archive>
void serialize(Archive& archive, rr::graphics::FontMetrics& metrics) {
    archive(
        cereal::make_nvp("em_size", metrics.emSize),
        cereal::make_nvp("ascender", metrics.ascender),
        cereal::make_nvp("descender", metrics.descender),
        cereal::make_nvp("line_height", metrics.lineHeight),
        cereal::make_nvp("underline_y", metrics.underlineY),
        cereal::make_nvp("underline_thickness", metrics.underlineThickness));
}

template <typename Archive>
void serialize(Archive& archive, rr::graphics::AtlasMetrics& metrics) {
    archive(cereal::make_nvp("type", metrics.type),
            cereal::make_nvp("distance_range", metrics.distanceRange),
            cereal::make_nvp("size", metrics.size),
            cereal::make_nvp("width", metrics.width),
            cereal::make_nvp("height", metrics.height),
            cereal::make_nvp("y_origin", metrics.yOrigin));
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
    archive(cereal::make_nvp("codepoint", metrics.codepoint),
            cereal::make_nvp("advance", metrics.advance),
            cereal::make_nvp("plane_bounds", metrics.planeBounds),
            cereal::make_nvp("atlas_bounds", metrics.atlasBounds));
}

template <typename Archive>
void serialize(Archive& archive, rr::graphics::FontSpecification& spec) {
    archive(cereal::make_nvp("name", spec.Name),
            cereal::make_nvp("id", spec.Id),
            cereal::make_nvp("atlas_id", spec.AtlasId),
            cereal::make_nvp("atlas_metrics", spec.AtlasMetrics),
            cereal::make_nvp("font_metrics", spec.FontMetrics),
            cereal::make_nvp("glyph_metrics", spec.GlyphMetrics));
}

template <typename Archive>
void serialize(Archive& archive, rr::graphics::ShaderSpecification& spec) {
    archive(cereal::make_nvp("name", spec.Name),
            cereal::make_nvp("id", spec.Id),
            cereal::make_nvp("vertex_filepath", spec.VertexFilepath),
            cereal::make_nvp("fragment_filepath", spec.FragmentFilepath));
}

}  // namespace rr::graphics

#endif  //! RR_LIBS_CORE_ASSET_MANAGER_H
