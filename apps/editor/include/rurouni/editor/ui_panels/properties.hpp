#ifndef RR_EDITOR_UI_PANELS_PROPERTIES_H
#define RR_EDITOR_UI_PANELS_PROPERTIES_H

#include "rurouni/editor/state.hpp"
#include "rurouni/editor/ui_items/property_tab.hpp"

#include "rurouni/core/asset_manager.hpp"

namespace rr::editor::ui {

class PropertiesPanel {
   public:
    static void draw(UIState& state,
                     entt::registry& registry,
                     core::AssetManager& assetManager);

    static void add_tab(std::unique_ptr<PropertyTab> tab);

   private:
    static std::vector<std::unique_ptr<PropertyTab>> m_Tabs;
    static int m_ForceTabIndexOpen;
};

}  // namespace rr::editor::ui

#endif  // !RR_EDITOR_UI_PANELS_PROPERTIES_H
