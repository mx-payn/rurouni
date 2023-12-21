#include "rurouni/editor/ui_panels/asset_manager.hpp"
#include "rurouni/editor/state.hpp"
#include "rurouni/editor/ui_panels/properties.hpp"

#include "rurouni/core/asset_manager.hpp"
#include "rurouni/system/filesystem.hpp"

#include <imgui/imgui.h>
#include <utility>

namespace rr::editor::ui {

void AssetManager::draw(UIState& state, core::AssetManager& assetManager) {
    if (!state.ShowPanelAssets)
        return;

    ImGui::Begin("Assets", &state.ShowPanelAssets);

    /*
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5, 0.5, 0.5, 0.8));
    // size_t image = state.Icons.FileImport->get_renderer_id();
    // if (ImGui::ImageButton((ImTextureID)image,
    //                        {(float)state.IconSize.x,
    (float)state.IconSize.y},
    //                        {0, 1}, {1, 0})) {
    //     state.ShowModalImportAsset = true;
    // }
    // ImGui::SetItemTooltip("Open import asset modal");
    ImGui::PopStyleColor(2);

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5, 0.5, 0.5, 0.8));
    // image = state.Icons.Save->get_renderer_id();
    // if (ImGui::ImageButton((ImTextureID)image,
    //                        {(float)state.IconSize.x,
    (float)state.IconSize.y},
    //                        {0, 1}, {1, 0})) {
    //     assetManager.write_asset_configuration();
    // }
    // ImGui::SetItemTooltip("Save asset specifications");
    ImGui::PopStyleColor(2);

    ImGui::Separator();
*/
    ImGui::BeginChild("contents");

    if (ImGui::CollapsingHeader("Textures")) {
        for (auto& [textureId, textureSpec] :
             assetManager.get_texture_registry()) {
            ImGui::PushID(textureId.to_string().append("##textures").c_str());
            if (ImGui::Selectable(textureSpec.Name.c_str())) {
                ui::PropertiesPanel::add_tab(std::make_unique<TextureProperty>(
                    textureId, textureSpec.Name));
            }

            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("TEXTURE_SPECIFICATION", &textureSpec,
                                          sizeof(textureSpec));

                ImGui::EndDragDropSource();
            }
            ImGui::PopID();
        }
    }

    if (ImGui::CollapsingHeader("Sprites")) {
        std::unordered_map<UUID,
                           std::unordered_map<UUID, core::SpriteSpecification*>>
            subTextureMap;

        for (auto& [spriteId, spriteSpec] :
             assetManager.get_sprite_registry()) {
            subTextureMap[spriteSpec.TextureId][spriteId] = &spriteSpec;
        }

        for (auto& [textureId, spriteEntry] : subTextureMap) {
            ImGui::PushID(textureId.to_string().append("##sprites").c_str());
            auto& textureSpec = assetManager.get_texture_registry()[textureId];

            auto flags = ImGuiTreeNodeFlags_SpanFullWidth;
            if (ImGui::TreeNodeEx(textureSpec.Name.c_str(), flags)) {
                for (auto& [spriteId, spriteSpecPtr] : spriteEntry) {
                    ImGui::PushID(
                        spriteId.to_string().append("##sprites").c_str());
                    if (ImGui::Selectable(spriteSpecPtr->Name.c_str())) {
                        ui::PropertiesPanel::add_tab(
                            std::make_unique<SpriteProperty>(
                                spriteId, spriteSpecPtr->TextureId,
                                spriteSpecPtr->Name));
                    }

                    if (ImGui::BeginDragDropSource()) {
                        ImGui::SetDragDropPayload("SPRITE_SPECIFICATION",
                                                  spriteSpecPtr,
                                                  sizeof(*spriteSpecPtr));

                        ImGui::EndDragDropSource();
                    }
                    ImGui::PopID();
                }

                ImGui::TreePop();
            }

            ImGui::PopID();
        }
    }

    if (ImGui::CollapsingHeader("Shaders")) {
        for (auto& [k, v] : assetManager.get_shader_registry()) {
            ImGui::Text("%s", v.Name.c_str());
        }
    }

    if (ImGui::CollapsingHeader("Fonts")) {
        for (auto& [k, v] : assetManager.get_font_registry()) {
            ImGui::Text("%s", v.Name.c_str());
        }
    }

    ImGui::EndChild();

    ImGui::End();
}

}  // namespace rr::editor::ui
