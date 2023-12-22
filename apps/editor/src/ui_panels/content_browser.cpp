#include "rurouni/editor/ui_panels/content_browser.hpp"
#include "rurouni/editor/state.hpp"

#include "rurouni/editor/logger.hpp"
#include "rurouni/system/filesystem.hpp"

#include <imgui/imgui.h>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace rr::editor::ui {

void ContentBrowser::draw(UIState& state, const system::Path& rootDir) {
    static system::Path currentDir = rootDir;

    ImGui::Begin("Content Browser", &state.ShowPanelContentBrowser);

    ImGui::BeginDisabled(currentDir == rootDir);
    if (ImGui::Button("back")) {
        currentDir = currentDir.parent_path();
    }
    ImGui::EndDisabled();

    ImGui::Separator();

    std::vector<system::directory_entry> entries;

    for (auto& p : system::directory_iterator(currentDir)) {
        if (p.is_directory()) {
            entries.push_back(p);
        }
    }

    for (auto& p : system::directory_iterator(currentDir)) {
        if (p.is_regular_file()) {
            entries.push_back(p);
        }
    }

    ImVec2 availableRegion = ImGui::GetContentRegionAvail();
    float thumbSizeX = state.ThumbnailSize.x;
    float thumbPadding = state.ThumbnailPadding;

    int rowAmount =
        std::floor(availableRegion.x / (thumbSizeX + thumbPadding * 2));

    ImGuiTableFlags flags = ImGuiTableFlags_PadOuterX;
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,
                        ImVec2(thumbPadding, thumbPadding / 2.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5, 0.5, 0.5, 0.8));
    if (ImGui::BeginTable("contents", std::clamp(rowAmount, 1, 511), flags)) {
        for (auto& p : entries) {
            ImGui::TableNextColumn();
            system::Path path = p.path();
            std::string relativeRootName =
                system::relative_root_name(path, rootDir);
            size_t image;
            if (p.is_directory()) {
                require(state.Icons.Folder->get_renderer_id().has_value(),
                        "folder icon not loaded");
                image = state.Icons.Folder->get_renderer_id().value();
                ImGui::ImageButton((ImTextureID)image,
                                   {(float)state.ThumbnailSize.x,
                                    (float)state.ThumbnailSize.y},
                                   {0, 0}, {1, 1});

                if (ImGui::IsItemHovered() &&
                    ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    currentDir = path;
                }
            } else if (p.is_regular_file()) {
                require(state.Icons.File->get_renderer_id().has_value(),
                        "file icon not loaded");
                image = state.Icons.File->get_renderer_id().value();
                ImGui::ImageButton((ImTextureID)image,
                                   {(float)state.ThumbnailSize.x,
                                    (float)state.ThumbnailSize.y},
                                   {0, 0}, {1, 1});

                if (relativeRootName == "scenes") {
                    const char* pathStr = path.c_str();
                    if (ImGui::BeginDragDropSource()) {
                        ImGui::SetDragDropPayload(
                            "CONTENT_BROWSER_SCENE_PATH", pathStr,
                            strlen(pathStr) + sizeof(char));

                        ImGui::EndDragDropSource();
                    }
                }
            }
            ImGui::TextWrapped("%s", path.filename().c_str());
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();

    ImGui::End();
}

}  // namespace rr::editor::ui
