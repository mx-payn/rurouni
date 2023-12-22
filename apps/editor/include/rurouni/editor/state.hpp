#ifndef RR_EDITOR_STATE_H
#define RR_EDITOR_STATE_H

#include "rurouni/graphics/texture.hpp"
#include "rurouni/math/vec.hpp"
#include "rurouni/system/filesystem.hpp"

#include <entt/entt.hpp>

#include <optional>
#include <string>

namespace rr::editor {

struct ModuleHistoryItem {
    std::string Name;
    system::Path RootPath;
};

struct UIState {
    math::ivec2 SceneViewportSize = math::ivec2(1.0f);

    std::optional<entt::entity> m_SelectedEntity = {};

    // panels / modals
    bool ShowModuleCreateModal = false;
    bool ShowPanelSceneViewport = true;
    bool ShowPanelAssets = true;
    bool ShowPanelProperties = true;
    bool ShowPanelScene = true;
    bool ShowPanelContentBrowser = true;

    // icons
    struct {
        std::shared_ptr<graphics::Texture> File;
        std::shared_ptr<graphics::Texture> FileImport;
        std::shared_ptr<graphics::Texture> Folder;
        std::shared_ptr<graphics::Texture> Save;
    } Icons;
    math::ivec2 ThumbnailSize = {128, 128};
    float ThumbnailPadding = 16.0f;
    math::ivec2 IconSize = {32, 32};
    float IconPadding = 4.0f;
};

}  // namespace rr::editor

/// serialization
#include <cereal/cereal.hpp>

namespace rr::editor {

template <class Archive>
void serialize(Archive& archive, rr::editor::ModuleHistoryItem& p) {
    archive(cereal::make_nvp("name", p.Name),
            cereal::make_nvp("root_path", p.RootPath));
}

}  // namespace rr::editor

#endif  // !RR_EDITOR_STATE_H
