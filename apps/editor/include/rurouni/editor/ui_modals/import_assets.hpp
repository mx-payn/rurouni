#ifndef RR_EDITOR_UI_MODALS_IMPORT_ASSET_H
#define RR_EDITOR_UI_MODALS_IMPORT_ASSET_H

#include "rurouni/editor/state.hpp"

#include "rurouni/core/asset_manager.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/types/uuid.hpp"

#include <unordered_map>

namespace rr::editor::ui {

class ImportTextureTab {
   public:
    ImportTextureTab() = default;
    ~ImportTextureTab() = default;

    void draw(core::AssetManager& assetManager);

    core::TextureSpecification& get_texture_spec() { return m_TextureSpec; }

   private:
    core::TextureSpecification m_TextureSpec;
};

class ImportSpriteTab {
   public:
    ImportSpriteTab() = default;
    ~ImportSpriteTab() = default;

    void draw(core::AssetManager& assetManager);

    std::unordered_map<int, core::SpriteSpecification>& get_marked_sprites() {
        return m_MarkedSpritesMap;
    }

   private:
    void get_existing_sprites(core::AssetManager& assetManager);

   private:
    std::unordered_map<int, core::SpriteSpecification> m_MarkedSpritesMap;
    std::unordered_map<int, core::SpriteSpecification> m_ExistingSpritesMap;

    std::optional<core::TextureSpecification> m_SelectedTextureSpec;
    std::optional<core::SpriteSpecification> m_SelectedSpriteSpec;
};

class ImportAssetsModal {
   public:
    ImportAssetsModal(
        std::function<void(core::TextureSpecification&)> importTextureFunc,
        std::function<void(std::unordered_map<int, core::SpriteSpecification>&)>
            importSpritesFunc,
        std::function<void(core::ShaderSpecification&)> importShaderFunc,
        std::function<void(core::FontSpecification&)> importFontFunc);

    void draw(UIState& state, core::AssetManager& assetManager);

   private:
    void draw_import_shader(core::AssetManager& assetManager);
    void draw_import_font(core::AssetManager& assetManager);

    void reset_state();

   private:
    core::AssetType m_SelectedTab = core::AssetType::Texture;

    std::optional<core::ShaderSpecification> m_SelectedShaderSpec;
    std::optional<core::FontSpecification> m_SelectedFontSpec;

    std::function<void(core::TextureSpecification&)> m_ImportTextureFunc;
    std::function<void(std::unordered_map<int, core::SpriteSpecification>&)>
        m_ImportSpritesFunc;
    std::function<void(core::ShaderSpecification&)> m_ImportShaderFunc;
    std::function<void(core::FontSpecification&)> m_ImportFontFunc;

    std::unique_ptr<ImportTextureTab> m_TextureTab;
    std::unique_ptr<ImportSpriteTab> m_SpriteTab;
};

}  // namespace rr::editor::ui

#endif  // !RR_EDITOR_UI_MODALS_IMPORT_ASSET_H
