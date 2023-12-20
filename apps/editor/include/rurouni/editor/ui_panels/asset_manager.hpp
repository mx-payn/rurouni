#ifndef RR_EDITOR_UI_PANELS_ASSET_MANAGER_H
#define RR_EDITOR_UI_PANELS_ASSET_MANAGER_H

#include "rurouni/editor/state.hpp"

#include "rurouni/core/asset_manager.hpp"

namespace rr::editor::ui {

class AssetManager {
   public:
    static void draw(UIState& state, core::AssetManager& assetManager);
};

}  // namespace rr::editor::ui

#endif  // !RR_EDITOR_PANELS_ASSET_MANAGER_H
