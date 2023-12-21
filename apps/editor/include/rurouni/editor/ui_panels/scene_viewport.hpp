#ifndef RR_EDITOR_PANELS_SCENE_VIEWPORT_H
#define RR_EDITOR_PANELS_SCENE_VIEWPORT_H

#include "entt/entity/entity.hpp"
#include "rurouni/core/scene.hpp"
#include "rurouni/editor/state.hpp"

namespace rr::editor::ui {

class SceneViewportPanel {
   public:
    static void draw(UIState& state,
                     core::Scene& scene,
                     std::function<void()> drawSceneFn,
                     std::function<void(const system::Path&)> changeSceneFn);

    static int get_hovered_entity() { return m_HoveredEntity; }

   private:
    static uint32_t m_HoveredEntity;
};

}  // namespace rr::editor::ui

#endif
