#ifndef RR_LIBS_CORE_ASSET_MANAGER_H
#define RR_LIBS_CORE_ASSET_MANAGER_H

#include "rurouni/error.hpp"
#include "rurouni/graphics/font.hpp"
#include "rurouni/graphics/shader.hpp"
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
#include "cereal/types/map.hpp"
#include "cereal/types/optional.hpp"

namespace rr::core {

enum class AssetType { Texture, Sprite, Shader, Font };

struct SpriteSpecification {
    std::string Name;
    UUID Id;
    UUID TextureId;
    math::ivec2 Cell_Idx;
    math::ivec2 CellSpread = math::ivec2(1.0f);

    // std::array<math::vec2, 4> CellCoords_UV;
};
struct TextureSpecification {
    std::string Name;
    UUID Id;
    system::Path Filepath;

    // sprites
    std::optional<math::ivec2> SpriteCount;
    math::vec2 SpriteSize_UV = math::vec2(0.0f);
};
struct ShaderSpecification {
    std::string Name;
    UUID Id;
    system::Path VertexSourcePath;
    system::Path FragmentSourcePath;
};
struct FontSpecification {
    std::string Name;
    UUID Id;
    system::Path Filepath;

    graphics::FontMetrics FontMetrics;
    graphics::AtlasMetrics AtlasMetrics;
    std::map<graphics::unicode_t, graphics::GlyphMetrics> GlyphMetrics;
};

class AssetManager {
   public:
    AssetManager(const system::Path& rootDir);
    ~AssetManager();

    std::optional<Error> read_asset_configuration();
    std::optional<Error> write_asset_configuration();

    bool load_texture(const UUID& uuid);
    bool load_shader(const UUID& uuid);

    void register_texture(const TextureSpecification& spec);
    void register_sprite(const SpriteSpecification& spec);
    void register_font(const FontSpecification& spec);
    void register_shader(const ShaderSpecification spec);

    void unload_texture(const UUID& uuid);
    void unload_shader(const UUID& uuid);

    std::weak_ptr<graphics::Texture> get_texture(const UUID& uuid);
    std::weak_ptr<graphics::Shader> get_shader(const UUID& uuid);

    std::unordered_map<UUID, TextureSpecification>& get_texture_registry() {
        return m_TextureRegistry;
    }

    std::unordered_map<UUID, SpriteSpecification>& get_sprite_registry() {
        return m_SpriteRegistry;
    }
    std::unordered_map<UUID, ShaderSpecification>& get_shader_registry() {
        return m_ShaderRegistry;
    }
    std::unordered_map<UUID, FontSpecification>& get_font_registry() {
        return m_FontRegistry;
    }

   private:
    system::Path m_RootDir;

    std::unordered_map<UUID, TextureSpecification> m_TextureRegistry;
    std::unordered_map<UUID, SpriteSpecification> m_SpriteRegistry;
    std::unordered_map<UUID, ShaderSpecification> m_ShaderRegistry;
    std::unordered_map<UUID, FontSpecification> m_FontRegistry;

    std::unordered_map<UUID, std::shared_ptr<graphics::Texture>> m_TextureCache;
    std::unordered_map<UUID, std::shared_ptr<graphics::Shader>> m_ShaderCache;
};

template <typename Archive>
void serialize(Archive& archive, rr::core::TextureSpecification& spec) {
    archive(cereal::make_nvp("name", spec.Name),
            cereal::make_nvp("id", spec.Id),
            cereal::make_nvp("filepath", spec.Filepath),
            cereal::make_nvp("sprite_count", spec.SpriteCount));

    // TODO put into seperate load function
    if (spec.SpriteCount.has_value()) {
        spec.SpriteSize_UV =
            math::vec2(1.0f / spec.SpriteCount->x, 1.0f / spec.SpriteCount->y);
    }
}

template <typename Archive>
void serialize(Archive& archive, rr::core::SpriteSpecification& spec) {
    archive(cereal::make_nvp("name", spec.Name),
            cereal::make_nvp("id", spec.Id),
            cereal::make_nvp("texture_id", spec.TextureId),
            cereal::make_nvp("cell_index", spec.Cell_Idx),
            cereal::make_nvp("cell_spread", spec.CellSpread));
}

template <typename Archive>
void serialize(Archive& archive, rr::core::ShaderSpecification& spec) {
    archive(cereal::make_nvp("name", spec.Name),
            cereal::make_nvp("id", spec.Id),
            cereal::make_nvp("vertex_source_path", spec.VertexSourcePath),
            cereal::make_nvp("fragment_source_path", spec.FragmentSourcePath));
}

template <typename Archive>
void serialize(Archive& archive, rr::core::FontSpecification& spec) {
    archive(cereal::make_nvp("name", spec.Name),
            cereal::make_nvp("id", spec.Id),
            cereal::make_nvp("filepath", spec.Filepath),
            cereal::make_nvp("font_metrics", spec.FontMetrics),
            cereal::make_nvp("atlas_metrics", spec.AtlasMetrics),
            cereal::make_nvp("glyph_metrics", spec.GlyphMetrics));
}

}  // namespace rr::core

#endif  //! RR_LIBS_CORE_ASSET_MANAGER_H
