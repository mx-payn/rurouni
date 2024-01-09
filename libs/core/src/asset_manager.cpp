#include "rurouni/graphics/sprite.hpp"
#include "rurouni/graphics/spritesheet.hpp"
#include "rurouni/graphics/texture.hpp"
#include "rurouni/pch.hpp"

#include "rurouni/core/asset_manager.hpp"
#include "rurouni/core/logger.hpp"
#include "rurouni/system/filesystem.hpp"

#include "cereal/archives/json.hpp"
#include "cereal/cereal.hpp"
#include "cereal/types/unordered_map.hpp"

#include <limits>
#include <memory>
#include <optional>

namespace rr::core {

AssetManager::AssetManager(const system::Path& rootDir) : m_RootDir(rootDir) {
    graphics::DataTextureSpecification textureSpec;
    textureSpec.Name = "DEFAULT_TEXTURE";
    textureSpec.Id = DEFAULT_TEXTURE_ID;
    textureSpec.Size = math::ivec2(1.0f);
    textureSpec.DataFormat = graphics::TextureDataFormat::RGBA;
    textureSpec.PixelFormat = graphics::TexturePixelFormat::RGBA8;
    uint32_t textureData = 0xFF0000FF;
    textureSpec.set_data(&textureData, sizeof(textureData));
    auto texture = std::make_shared<graphics::Texture>(textureSpec);
    m_LoadedTextureCache[DEFAULT_TEXTURE_ID] = texture;

    graphics::SpritesheetSpecification sheetSpec;
    sheetSpec.Name = "DEFAULT_SPRITESHEET";
    sheetSpec.Id = DEFAULT_SPRITESHEET_ID;
    sheetSpec.CellCount = math::ivec2(std::numeric_limits<int32_t>::max());
    auto spritesheetPtr =
        std::make_shared<graphics::Spritesheet>(sheetSpec, texture);
    m_LoadedTextureCache[DEFAULT_SPRITESHEET_ID] = spritesheetPtr;

    graphics::SpriteSpecification spriteSpec;
    spriteSpec.Name = "DEFAULT_SPRITE";
    spriteSpec.Id = DEFAULT_SPRITE_ID;
    spriteSpec.SpritesheetId = DEFAULT_SPRITESHEET_ID;
    m_LoadedTextureCache[DEFAULT_SPRITE_ID] =
        std::make_shared<graphics::Sprite>(spriteSpec, spritesheetPtr);

    // TODO default font
    // TODO default shader
}
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
        inputArchive(cereal::make_nvp("image_textures", m_ImageTextureDatabase),
                     cereal::make_nvp("spritesheets", m_SpritesheetDatabase),
                     cereal::make_nvp("sprites", m_SpriteDatabase),
                     cereal::make_nvp("fonts", m_FontDatabase));
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
        outputArchive(
            cereal::make_nvp("image_textures", m_ImageTextureDatabase),
            cereal::make_nvp("spritesheets", m_SpritesheetDatabase),
            cereal::make_nvp("sprites", m_SpriteDatabase),
            cereal::make_nvp("fonts", m_FontDatabase));
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

void AssetManager::load_image_texture(const UUID& uuid) {
    // texture already loaded
    if (m_LoadedTextureCache.find(uuid) != m_LoadedTextureCache.end())
        return;

    if (m_ImageTextureDatabase.find(uuid) == m_ImageTextureDatabase.end()) {
        error("image texture spec not found. mapping default texture into: {}",
              uuid);
        m_LoadedTextureCache[uuid] = m_LoadedTextureCache[DEFAULT_TEXTURE_ID];
        return;
    }

    auto& spec = m_ImageTextureDatabase[uuid];
    m_LoadedTextureCache[uuid] = std::make_shared<graphics::Texture>(spec);
    // TODO some kind of way to catch errors on construction
}

void AssetManager::load_spritesheet(const UUID& uuid) {
    // texture already loaded
    if (m_LoadedTextureCache.find(uuid) != m_LoadedTextureCache.end())
        return;

    if (m_SpritesheetDatabase.find(uuid) == m_SpritesheetDatabase.end()) {
        error(
            "spritesheet spec not found. loading default spritesheet into: {}",
            uuid);
        m_LoadedTextureCache[uuid] =
            m_LoadedTextureCache[DEFAULT_SPRITESHEET_ID];
        return;
    }

    auto& spec = m_SpritesheetDatabase[uuid];
    load_image_texture(spec.TextureId);

    auto texture = m_LoadedTextureCache[spec.TextureId];
    m_LoadedTextureCache[uuid] =
        std::make_shared<graphics::Spritesheet>(spec, texture);
    // TODO some kind of way to catch errors on construction
}

void AssetManager::load_sprite(const UUID& uuid) {
    // texture already loaded
    if (m_LoadedTextureCache.find(uuid) != m_LoadedTextureCache.end())
        return;

    if (m_SpriteDatabase.find(uuid) == m_SpriteDatabase.end()) {
        error("sprite spec not found. mapping default sprite into: {}", uuid);
        m_LoadedTextureCache[uuid] = m_LoadedTextureCache[DEFAULT_SPRITE_ID];
        return;
    }

    auto& spec = m_SpriteDatabase[uuid];
    load_spritesheet(spec.SpritesheetId);

    auto spritesheet = std::static_pointer_cast<graphics::Spritesheet>(
        m_LoadedTextureCache[spec.SpritesheetId]);
    m_LoadedTextureCache[uuid] =
        std::make_shared<graphics::Sprite>(spec, spritesheet);
    // TODO some kind of way to catch errors on construction
}

void AssetManager::load_font(const UUID& uuid) {
    // texture already loaded
    if (m_LoadedTextureCache.find(uuid) != m_LoadedTextureCache.end())
        return;

    if (m_FontDatabase.find(uuid) == m_FontDatabase.end()) {
        error("font spec not found. mapping default font into: {}", uuid);
        m_LoadedTextureCache[uuid] = m_LoadedTextureCache[DEFAULT_FONT_ID];
        return;
    }

    auto& spec = m_FontDatabase[uuid];
    load_image_texture(spec.AtlasId);

    auto fontAtlas = std::static_pointer_cast<graphics::Font>(
        m_LoadedTextureCache[spec.AtlasId]);
    m_LoadedTextureCache[uuid] =
        std::make_shared<graphics::Font>(spec, fontAtlas);
    // TODO some kind of way to catch errors on construction
}

void AssetManager::load_shader(const UUID& uuid) {
    // shader already loaded
    if (m_LoadedShaderCache.find(uuid) != m_LoadedShaderCache.end())
        return;

    if (m_ShaderDatabase.find(uuid) == m_ShaderDatabase.end()) {
        error("shader spec not found. mapping default shader into: {}", uuid);
        m_LoadedShaderCache[uuid] = m_LoadedShaderCache[DEFAULT_SHADER_ID];
        return;
    }

    auto& spec = m_ShaderDatabase[uuid];
    m_LoadedShaderCache[uuid] = std::make_shared<graphics::Shader>(spec);
    // TODO some kind of way to catch errors on construction
}

void AssetManager::register_image_texture(
    const graphics::ImageTextureSpecification& spec) {
    if (m_ImageTextureDatabase.find(spec.Id) != m_ImageTextureDatabase.end()) {
        warn("overwriting a texture specification in the registry. id: {}",
             spec.Id);
    }

    m_ImageTextureDatabase[spec.Id] = spec;
}
void AssetManager::register_spritesheet(
    const graphics::SpritesheetSpecification& spec) {
    if (m_SpritesheetDatabase.find(spec.Id) != m_SpritesheetDatabase.end()) {
        warn("overwriting a texture specification in the registry. id: {}",
             spec.Id);
    }

    m_SpritesheetDatabase[spec.Id] = spec;
}

void AssetManager::register_sprite(const graphics::SpriteSpecification& spec) {
    if (m_SpriteDatabase.find(spec.Id) != m_SpriteDatabase.end()) {
        warn("overwriting a texture specification in the registry. id: {}",
             spec.Id);
    }

    m_SpriteDatabase[spec.Id] = spec;
}
void AssetManager::register_font(const graphics::FontSpecification& spec) {
    if (m_FontDatabase.find(spec.Id) != m_FontDatabase.end()) {
        warn("overwriting a texture specification in the registry. id: {}",
             spec.Id);
    }

    m_FontDatabase[spec.Id] = spec;
}

void AssetManager::register_shader(const graphics::ShaderSpecification spec) {
    if (m_ShaderDatabase.find(spec.Id) != m_ShaderDatabase.end()) {
        warn("overwriting a shader specification in the registry. id: {}",
             spec.Id);
    }

    m_ShaderDatabase[spec.Id] = spec;
}

std::weak_ptr<graphics::Texture> AssetManager::get_texture(const UUID& uuid) {
    // TODO this gets called for each texture each frame
    //      try to not check each call
    if (m_LoadedTextureCache.find(uuid) == m_LoadedTextureCache.end()) {
        if (m_ImageTextureDatabase.find(uuid) != m_ImageTextureDatabase.end()) {
            load_image_texture(uuid);
        } else if (m_SpritesheetDatabase.find(uuid) !=
                   m_SpritesheetDatabase.end()) {
            load_spritesheet(uuid);
        } else if (m_SpriteDatabase.find(uuid) != m_SpriteDatabase.end()) {
            load_sprite(uuid);
        } else {
            error(
                "failed fetching texture [{}], replacing it with default "
                "texture...",
                uuid);
            m_LoadedTextureCache[uuid] =
                m_LoadedTextureCache[DEFAULT_TEXTURE_ID];
        }
    }

    return m_LoadedTextureCache[uuid];
}

std::weak_ptr<graphics::Shader> AssetManager::get_shader(const UUID& uuid) {
    if (m_LoadedShaderCache.find(uuid) == m_LoadedShaderCache.end()) {
        error("failed loading shader [{}], replacing it with default shader...",
              uuid);
        m_LoadedShaderCache[uuid] = m_LoadedShaderCache[DEFAULT_TEXTURE_ID];
    }

    return m_LoadedShaderCache[uuid];
}

void AssetManager::unload_texture(const UUID& uuid) {
    m_LoadedTextureCache.erase(uuid);
}

void AssetManager::unload_shader(const UUID& uuid) {
    m_LoadedShaderCache.erase(uuid);
}

bool AssetManager::has_image_texture_specification(const UUID& uuid) {
    if (m_ImageTextureDatabase.find(uuid) == m_ImageTextureDatabase.end()) {
        return false;
    }

    return true;
}

bool AssetManager::texture_is_loaded(const UUID& uuid) {
    if (m_LoadedTextureCache.find(uuid) == m_LoadedTextureCache.end()) {
        return false;
    }

    return true;
}

}  // namespace rr::core
