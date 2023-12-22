#include "rurouni/editor/ui_modals/import_assets.hpp"
#include "rurouni/editor/state.hpp"

#include "rurouni/core/asset_manager.hpp"
#include "rurouni/math/vec.hpp"
#include "rurouni/system/command.hpp"
#include "rurouni/types/uuid.hpp"

#include <imgui/imgui.h>
#include <strings.h>
#include <unordered_map>

namespace rr::editor::ui {

ImportAssetsModal::ImportAssetsModal(
    std::function<void(core::TextureSpecification&)> importTextureFunc,
    std::function<void(std::unordered_map<int, core::SpriteSpecification>&)>
        importSpritesFunc,
    std::function<void(core::ShaderSpecification&)> importShaderFunc,
    std::function<void(core::FontSpecification&)> importFontFunc)
    : m_ImportTextureFunc(importTextureFunc),
      m_ImportSpritesFunc(importSpritesFunc),
      m_ImportShaderFunc(importShaderFunc),
      m_ImportFontFunc(importFontFunc) {}

void ImportAssetsModal::draw(UIState& state, core::AssetManager& assetManager) {
    // draw nothing if this modal is turned off
    if (!state.ShowModalImportAsset)
        return;

    ImGui::OpenPopup("Import Asset");

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Import Asset", &state.ShowModalImportAsset,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        // Tab bar
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("Asset Type", tab_bar_flags)) {
            if (ImGui::BeginTabItem("Texture")) {
                m_SelectedTab = core::AssetType::Texture;
                draw_import_texture(assetManager);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Sprite")) {
                m_SelectedTab = core::AssetType::Sprite;
                draw_import_sprites(assetManager);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Shader")) {
                m_SelectedTab = core::AssetType::Shader;
                draw_import_shader(assetManager);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Font")) {
                m_SelectedTab = core::AssetType::Font;
                draw_import_font(assetManager);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        // ok button
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            // execute import function for current selected asset type
            switch (m_SelectedTab) {
                case core::AssetType::Texture:
                    if (m_SelectedTextureSpec.has_value())
                        m_ImportTextureFunc(m_SelectedTextureSpec.value());
                    break;
                case core::AssetType::Sprite:
                    if (!m_MarkedSpritesMap.empty())
                        m_ImportSpritesFunc(m_MarkedSpritesMap);
                    break;
                case core::AssetType::Shader:
                    if (m_SelectedShaderSpec.has_value())
                        m_ImportShaderFunc(m_SelectedShaderSpec.value());
                    break;
                case core::AssetType::Font:
                    if (m_SelectedFontSpec.has_value())
                        m_ImportFontFunc(m_SelectedFontSpec.value());
                    break;
            }

            reset_state();
            // reset global state
            state.ShowModalImportAsset = false;

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        // cancel button
        if (ImGui::Button("Cancel")) {
            reset_state();
            // reset global state
            state.ShowModalImportAsset = false;

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void ImportAssetsModal::reset_state() {
    // reset local state
    m_SelectedTab = core::AssetType::Texture;
    m_SelectedTextureSpec = {};
    m_SelectedSpriteSpec = {};
    m_MarkedSpritesMap.clear();
    m_ExistingSpritesMap.clear();
    m_SelectedShaderSpec = {};
    m_SelectedFontSpec = {};
}

void ImportAssetsModal::get_existing_sprites(core::AssetManager& assetManager) {
    auto& spriteRegistry = assetManager.get_sprite_registry();

    // copy existing specs into local specMap
    for (auto& [k, v] : spriteRegistry) {
        // TODO make != operator for UUID
        if (!(v.TextureId == m_SelectedTextureSpec->Id))
            continue;

        // calculate single digit index inside texture
        int index = (v.Cell_Idx.y * m_SelectedTextureSpec->SpriteCount->x) +
                    v.Cell_Idx.x;
        m_ExistingSpritesMap[index] = v;
    }
}

void ImportAssetsModal::draw_import_texture(core::AssetManager& assetManager) {
    static core::TextureSpecification spec;

    static char name[32] = "";
    if (ImGui::InputText("name", name, 32)) {
        spec.Name = std::string(name);
    }

    static char path[256] = "";
    if (ImGui::InputText("path", path, 256)) {
        spec.Filepath = std::string(path);
    }

    ImGui::SameLine();

    if (ImGui::Button("Open")) {
        std::string result = system::execute_command("zenity --file-selection");
        result.copy(path, result.size());
        spec.Filepath = result;
    }

    static math::ivec2 spriteCount = glm::ivec2(0.0f);
    if (ImGui::DragInt2("Sprite Count", math::value_ptr(spriteCount), 0.25f, 0,
                        std::numeric_limits<int32_t>::max())) {
        if (spriteCount == glm::ivec2(0.0f)) {
            spec.SpriteCount = {};
        } else {
            spec.SpriteCount = spriteCount;
        }
    }

    m_SelectedTextureSpec = spec;
}
void ImportAssetsModal::draw_import_shader(core::AssetManager& assetManager) {}
void ImportAssetsModal::draw_import_font(core::AssetManager& assetManager) {}

void ImportAssetsModal::draw_import_sprites(core::AssetManager& assetManager) {
    ImGui::BeginGroup();

    // texture selection
    std::string comboHint = m_SelectedTextureSpec.has_value()
                                ? m_SelectedTextureSpec->Name
                                : "Select Texture";
    if (ImGui::BeginCombo("Spritesheet", comboHint.c_str())) {
        for (const auto& [id, texSpec] : assetManager.get_texture_registry()) {
            // if texture has no sprites, dont show it
            if (!texSpec.SpriteCount.has_value())
                continue;

            const bool is_selected = (m_SelectedTextureSpec.has_value()) &&
                                     (m_SelectedTextureSpec->Id == id);
            if (ImGui::Selectable(texSpec.Name.c_str(), is_selected)) {
                m_SelectedTextureSpec = texSpec;
                get_existing_sprites(assetManager);
            }

            // Set the initial focus when opening the combo (scrolling +
            // keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    // sprite selection
    bool deleteAction = false;
    if (ImGui::BeginListBox("Marked Sprites")) {
        for (auto it = m_MarkedSpritesMap.begin();
             it != m_MarkedSpritesMap.end();) {
            ImGui::PushID(it->second.Id.to_string().c_str());

            const bool is_selected =
                (m_SelectedSpriteSpec.has_value()) &&
                (m_SelectedSpriteSpec->Id == it->second.Id);

            if (ImGui::Button("X")) {
                if (is_selected)
                    m_SelectedSpriteSpec = {};

                it = m_MarkedSpritesMap.erase(it);

                ImGui::PopID();
                continue;
            }

            ImGui::SameLine();

            if (ImGui::Selectable(it->second.Name.c_str(), is_selected)) {
                m_SelectedSpriteSpec = it->second;
            }

            // Set the initial focus when opening the combo (scrolling +
            // keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();

            ImGui::PopID();
            it++;
        }
        ImGui::EndListBox();
    }

    // draw sprite info if one is selected
    if (m_SelectedSpriteSpec.has_value()) {
        ImGui::Text("%s", m_SelectedSpriteSpec->Name.c_str());
    }

    ImGui::EndGroup();

    // draw nothing else, if there are no sprites in texture
    if (!m_SelectedTextureSpec.has_value())
        return;

    ImGui::SameLine();

    int spriteCountX = m_SelectedTextureSpec->SpriteCount->x;
    int spriteCountY = m_SelectedTextureSpec->SpriteCount->y;
    float spriteSizeX_uv = 1.0f / spriteCountX;
    float spriteSizeY_uv = 1.0f / spriteCountY;

    // draw image
    auto texture = assetManager.get_texture(m_SelectedTextureSpec->Id).lock();
    // TODO has_value() check?
    size_t textureId = texture->get_renderer_id().value();
    auto textureSize = texture->get_size();
    ImVec2 uv_min = ImVec2(0.0f, 0.0f);  // Top-left
    ImVec2 uv_max = ImVec2(1.0f, 1.0f);  // Lower-right
    ImVec2 region = ImGui::GetContentRegionAvail();
    float ar = (float)textureSize.x / (float)textureSize.y;

    // textureSize.x = region.x;
    // textureSize.y = region.x * ar;
    // TODO not-hardcoded values
    textureSize.x = 512;
    textureSize.y = 512;

    ImVec2 imagePosition = ImGui::GetCursorScreenPos();
    ImGui::SetNextItemAllowOverlap();
    ImGui::Image((void*)textureId, ImVec2(textureSize.x, textureSize.y), uv_min,
                 uv_max);

    // TODO not-hardcoded values
    float spriteSizeX = 512.0f / m_SelectedTextureSpec->SpriteCount->x;
    float spriteSizeY = 512.0f / m_SelectedTextureSpec->SpriteCount->y;

    float positionX = imagePosition.x;
    float positionY = imagePosition.y;

    // draw sprite grid
    for (int y = 0; y < spriteCountY; y++) {
        for (int x = 0; x < spriteCountX; x++) {
            int index = (y * spriteCountX) + x;
            bool isInExistingMap =
                m_ExistingSpritesMap.find(index) != m_ExistingSpritesMap.end();
            bool isInMarkedMap =
                m_MarkedSpritesMap.find(index) != m_MarkedSpritesMap.end();

            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
            if (isInExistingMap) {
                ImGui::PushStyleColor(ImGuiCol_Button,
                                      (ImVec4)ImColor(0.1f, 0.2f, 0.9f, 0.5f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                      (ImVec4)ImColor(0.2f, 0.1f, 0.8f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                      (ImVec4)ImColor(0.0f, 0.8f, 0.8f, 0.5f));
            } else if (isInMarkedMap) {
                ImGui::PushStyleColor(ImGuiCol_Button,
                                      (ImVec4)ImColor(0.1f, 0.85f, 0.1f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                      (ImVec4)ImColor(0.1f, 0.85f, 0.1f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                      (ImVec4)ImColor(0.0f, 0.8f, 0.8f, 0.5f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button,
                                      (ImVec4)ImColor(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                      (ImVec4)ImColor(0.2f, 0.5f, 0.2f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                      (ImVec4)ImColor(0.0f, 0.8f, 0.8f, 0.5f));
            }

            ImGui::SetCursorScreenPos(ImVec2(positionX, positionY));
            ImGui::PushID(fmt::format("{}{}", x, y).c_str());
            if (ImGui::Button("##button", ImVec2(spriteSizeX, spriteSizeY))) {
                if (!isInExistingMap && !isInMarkedMap) {  // add sprite to map
                    // flipped y index
                    int _y = spriteCountY - 1 - y;
                    core::SpriteSpecification spec;
                    spec.Name = fmt::format("x[{}]y[{}]", x, y);
                    spec.Id = UUID::create();
                    spec.Cell_Idx = math::ivec2(x, y);
                    spec.TextureId = m_SelectedTextureSpec->Id;
                    spec.CellSpread = math::ivec2(1.0f);

                    float uv_x = (float)x / spriteCountX;
                    float uv_y = (float)_y / spriteCountY;

                    // spec.TextureCoords_UV = {
                    //     math::vec2(uv_x, uv_y),
                    //     math::vec2(uv_x + spriteSizeX_uv, uv_y),
                    //     math::vec2(uv_x + spriteSizeX_uv,
                    //                uv_y + spriteSizeY_uv),
                    //     math::vec2(uv_x, uv_y + spriteSizeY_uv)};

                    m_MarkedSpritesMap[index] = spec;
                    m_SelectedSpriteSpec = spec;
                } else if (isInExistingMap) {
                    m_SelectedSpriteSpec = m_ExistingSpritesMap[index];
                } else if (isInMarkedMap) {
                    m_SelectedSpriteSpec = m_MarkedSpritesMap[index];
                }
            }
            ImGui::PopID();

            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar();

            positionX += spriteSizeX;
        }
        positionX = imagePosition.x;
        positionY += spriteSizeY;
    }
}

}  // namespace rr::editor::ui
