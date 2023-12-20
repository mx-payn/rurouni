#include "rurouni/graphics/sprite.hpp"
#include "rurouni/graphics/texture.hpp"
#include "rurouni/pch.hpp"

#include "rurouni/core/asset_manager.hpp"
#include "rurouni/core/logger.hpp"
#include "rurouni/system/filesystem.hpp"

#include "cereal/archives/json.hpp"
#include "cereal/cereal.hpp"
#include "cereal/types/unordered_map.hpp"

#include <memory>
#include <optional>

namespace rr::core {

AssetManager::AssetManager(const system::Path& rootDir) : m_RootDir(rootDir) {}
AssetManager::~AssetManager() {}

std::optional<Error> AssetManager::read_asset_configuration() {
    auto registryPath = m_RootDir / "assets.json";
    if (!system::exists(registryPath)) {
        error(
            "provided path to registry does not exist on the filesystem. path: "
            "{}",
            registryPath);
        return Error("read asset registry",
                     "provided path to registry does not exist on the "
                     "filesystem. path: {}",
                     registryPath);
    }

    if (!system::is_regular_file(registryPath)) {
        error(
            "provided path to registry does not lead to a regular file. path: "
            "{}",
            registryPath);
        return Error("read asset registry",
                     "provided path to registry does not lead to a regular "
                     "file. path: {}",
                     registryPath);
    }

    std::ifstream is(registryPath);
    try {
        cereal::JSONInputArchive inputArchive(is);
        inputArchive(cereal::make_nvp("textures", m_TextureRegistry),
                     cereal::make_nvp("sprites", m_SpriteRegistry),
                     cereal::make_nvp("shaders", m_ShaderRegistry),
                     cereal::make_nvp("fonts", m_FontRegistry));
    } catch (cereal::Exception e) {
        error("failed to deserialize asset registry. path: {}, cereal: {}",
              registryPath, e.what());
        return Error(
            "deserialize asset registry",
            "failed to deserialize asset registry. path: {}, cereal: {}",
            registryPath, e.what());
    }
    is.close();

    return std::nullopt;
}

std::optional<Error> AssetManager::write_asset_configuration() {
    auto registryPath = m_RootDir / "assets.json";
    if (system::exists(registryPath)) {
        if (system::is_regular_file(registryPath)) {
            warn(
                "path already exists on the filesystem. moving existing file "
                "to '.old' extension. path: {}",
                registryPath);
            system::rename(registryPath, registryPath.string().append(".old"));
        } else {
            error(
                "provided path exists as a non regular file on the filesystem. "
                "aborting the write operation. path: {}",
                registryPath);
            return Error("write asset registry",
                         "provided path exists as a non regular file on the "
                         "filesystem. aborting the write operation. path: {}",
                         registryPath);
        }
    }

    std::ofstream os(registryPath);
    try {
        cereal::JSONOutputArchive outputArchive(os);
        outputArchive(cereal::make_nvp("textures", m_TextureRegistry),
                      cereal::make_nvp("sprites", m_SpriteRegistry),
                      cereal::make_nvp("shaders", m_ShaderRegistry),
                      cereal::make_nvp("fonts", m_FontRegistry));
    } catch (cereal::Exception e) {
        error("failed to serialize asset registry. path: {}, cereal: {}",
              registryPath, e.what());
        return Error("serialize asset registry",
                     "failed to serialize asset registry. path: {}, cereal: {}",
                     registryPath, e.what());
    }
    os.close();

    return std::nullopt;
}

bool AssetManager::load_texture(const UUID& uuid) {
    // texture already loaded
    if (m_TextureCache.find(uuid) != m_TextureCache.end())
        return true;

    if (m_TextureRegistry.find(uuid) != m_TextureRegistry.end()) {
        auto& spec = m_TextureRegistry[uuid];
        m_TextureCache[spec.Id] = std::make_shared<graphics::Texture>(
            graphics::ImageTextureSpecification{m_RootDir / spec.Filepath,
                                                spec.Id});
        trace("loaded texture [{}][{}]", spec.Name, spec.Id);
        return true;
    } else if (m_SpriteRegistry.find(uuid) != m_SpriteRegistry.end()) {
        auto& spec = m_SpriteRegistry[uuid];
        if (!load_texture(spec.TextureId)) {
            return false;
        }
        auto& atlasSpec = m_TextureRegistry[spec.TextureId];
        auto atlasPtr = m_TextureCache[spec.TextureId];

        math::vec2 uvCoordsMin = {
            atlasSpec.SpriteSize_UV.x * spec.Cell_Idx.x,
            atlasSpec.SpriteSize_UV.y * spec.Cell_Idx.y,
        };
        math::vec2 uvCoordsMax = {uvCoordsMin.x + atlasSpec.SpriteSize_UV.x,
                                  uvCoordsMin.y + atlasSpec.SpriteSize_UV.y};
        std::array<math::vec2, 4> uvCoords = {
            math::vec2{uvCoordsMin.x, uvCoordsMin.y},
            math::vec2{uvCoordsMax.x, uvCoordsMin.y},
            math::vec2{uvCoordsMax.x, uvCoordsMax.y},
            math::vec2{uvCoordsMin.x, uvCoordsMax.y}};
        math::ivec2 spriteSize_PX =
            math::ivec2{atlasPtr->get_size().x / atlasSpec.SpriteCount->x,
                        atlasPtr->get_size().y / atlasSpec.SpriteCount->y};

        m_TextureCache[spec.Id] = std::make_shared<graphics::Sprite>(
            spec.Cell_Idx, m_TextureCache[spec.TextureId], spec.Id,
            spriteSize_PX, uvCoords);

        trace("loaded sprite name[{}]id[{}]index[{}]", spec.Name, spec.Id,
              spec.Cell_Idx);
        return true;
    } else if (m_FontRegistry.find(uuid) != m_FontRegistry.end()) {
        auto& spec = m_FontRegistry[uuid];
        m_TextureCache[uuid] = std::make_shared<graphics::Font>(
            graphics::ImageTextureSpecification{spec.Filepath, spec.Id},
            spec.FontMetrics, spec.AtlasMetrics, spec.GlyphMetrics);
        trace("loaded font name[{}]id[{}]", spec.Name, spec.Id);
        return true;
    }

    error("could not find a registry entry for texture. uuid: {}", uuid);
    return false;
}

bool AssetManager::load_shader(const UUID& uuid) {
    if (m_ShaderCache.find(uuid) != m_ShaderCache.end())
        return true;

    if (m_ShaderRegistry.find(uuid) == m_ShaderRegistry.end()) {
        warn("no registry entry found for shader. uuid: {}", uuid.to_string());
        return false;
    }

    auto spec = m_ShaderRegistry[uuid];
    m_ShaderCache[uuid] = std::make_shared<graphics::Shader>(
        m_RootDir / spec.VertexSourcePath, m_RootDir / spec.FragmentSourcePath);
    trace("loaded shader [{}][{}]", spec.Name, spec.Id.to_string());
    return true;
}

void AssetManager::register_texture(const TextureSpecification& spec) {
    if (m_TextureRegistry.find(spec.Id) != m_TextureRegistry.end()) {
        warn("overwriting a texture specification in the registry. id: {}",
             spec.Id);
    }

    m_TextureRegistry[spec.Id] = spec;
}

void AssetManager::register_sprite(const SpriteSpecification& spec) {
    if (m_SpriteRegistry.find(spec.Id) != m_SpriteRegistry.end()) {
        warn("overwriting a sprite specification in the registry. id: {}",
             spec.Id);
    }

    m_SpriteRegistry[spec.Id] = spec;
}

void AssetManager::register_font(const FontSpecification& spec) {
    if (m_FontRegistry.find(spec.Id) != m_FontRegistry.end()) {
        warn("overwriting a font specification in the registry. id: {}",
             spec.Id);
    }

    m_FontRegistry[spec.Id] = spec;
}

void AssetManager::register_shader(const ShaderSpecification spec) {
    if (m_ShaderRegistry.find(spec.Id) != m_ShaderRegistry.end()) {
        warn("overwriting a shader specification in the registry. id: {}",
             spec.Id);
    }

    m_ShaderRegistry[spec.Id] = spec;
}

std::weak_ptr<graphics::Texture> AssetManager::get_texture(const UUID& uuid) {
    if (!load_texture(uuid)) {
        return {};
    }

    return m_TextureCache[uuid];
}

std::weak_ptr<graphics::Shader> AssetManager::get_shader(const UUID& uuid) {
    if (!load_shader(uuid)) {
        return {};
    }

    return m_ShaderCache[uuid];
}

}  // namespace rr::core
