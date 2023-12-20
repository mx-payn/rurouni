#include "rurouni/editor/ui_panels/properties.hpp"
#include "rurouni/editor/state.hpp"
#include "rurouni/editor/ui_items/property_tab.hpp"

#include "rurouni/types/uuid.hpp"

#include <imgui/imgui.h>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace rr::editor::ui {

std::vector<std::unique_ptr<PropertyTab>> PropertiesPanel::m_Tabs;
int PropertiesPanel::m_ForceTabIndexOpen = -1;

void PropertiesPanel::add_tab(std::unique_ptr<PropertyTab> tab) {
    for (int i = 0; i < m_Tabs.size(); i++) {
        if (m_Tabs[i]->get_id() == tab->get_id()) {
            // TODO set tab open and return
            m_ForceTabIndexOpen = i;
            return;
        }
    }

    m_Tabs.push_back(std::move(tab));
}

void PropertiesPanel::draw(UIState& state,
                           entt::registry& registry,
                           core::AssetManager& assetManager) {
    ImGui::Begin("Properties", &state.ShowPanelProperties);

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable |
                                     ImGuiTabBarFlags_AutoSelectNewTabs |
                                     ImGuiTabBarFlags_FittingPolicyScroll |
                                     ImGuiTabBarFlags_TabListPopupButton;
    if (ImGui::BeginTabBar("Property Type", tab_bar_flags)) {
        for (int i = 0; i < m_Tabs.size(); i++) {
            ImGui::PushID(std::to_string(i).c_str());

            ImGuiTabItemFlags flags = 0;

            if (i == m_ForceTabIndexOpen) {
                flags = ImGuiTabItemFlags_SetSelected;
                m_ForceTabIndexOpen = -1;
            }

            bool opened = true;
            if (ImGui::BeginTabItem(m_Tabs[i]->get_name().c_str(), &opened,
                                    flags)) {
                m_Tabs[i]->draw(registry, assetManager);

                ImGui::EndTabItem();
            }

            if (!opened) {
                m_Tabs.erase(m_Tabs.begin() + i);
            }

            ImGui::PopID();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

}  // namespace rr::editor::ui
