#ifndef RR_EDITOR_UI_MODALS_IMPORT_ASSET_H
#define RR_EDITOR_UI_MODALS_IMPORT_ASSET_H

#include "rurouni/editor/state.hpp"

#include "rurouni/core/asset_manager.hpp"
#include "rurouni/types/uuid.hpp"

#include <unordered_map>

namespace rr::editor::ui {

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
    void draw_import_texture(core::AssetManager& assetManager);
    void draw_import_sprites(core::AssetManager& assetManager);
    void draw_import_shader(core::AssetManager& assetManager);
    void draw_import_font(core::AssetManager& assetManager);

    void reset_state();
    void get_existing_sprites(core::AssetManager& assetManager);

   private:
    core::AssetType m_SelectedTab = core::AssetType::Texture;

    std::optional<core::TextureSpecification> m_SelectedTextureSpec;
    std::optional<core::SpriteSpecification> m_SelectedSpriteSpec;
    std::optional<core::ShaderSpecification> m_SelectedShaderSpec;
    std::optional<core::FontSpecification> m_SelectedFontSpec;

    std::unordered_map<int, core::SpriteSpecification> m_MarkedSpritesMap;
    std::unordered_map<int, core::SpriteSpecification> m_ExistingSpritesMap;

    std::function<void(core::TextureSpecification&)> m_ImportTextureFunc;
    std::function<void(std::unordered_map<int, core::SpriteSpecification>&)>
        m_ImportSpritesFunc;
    std::function<void(core::ShaderSpecification&)> m_ImportShaderFunc;
    std::function<void(core::FontSpecification&)> m_ImportFontFunc;
};

}  // namespace rr::editor::ui

#endif  // !RR_EDITOR_UI_MODALS_IMPORT_ASSET_H
