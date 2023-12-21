#ifndef RR_EDITOR_UI_PANELS_SCENE_H
#define RR_EDITOR_UI_PANELS_SCENE_H

#include "rurouni/editor/state.hpp"

#include "rurouni/core/scene.hpp"

#include <entt/entt.hpp>

namespace rr::editor::ui {

class ScenePanel {
   public:
    static void draw(UIState& state,
                     core::Scene& scene,
                     std::function<void(const system::Path&)> changeSceneFn);

   private:
    static void draw_entity_node(UIState& state,
                                 entt::registry& registry,
                                 entt::entity entityId);
};

}  // namespace rr::editor::ui

#endif  // !RR_EDITOR_UI_PANELS_SCENE_H
