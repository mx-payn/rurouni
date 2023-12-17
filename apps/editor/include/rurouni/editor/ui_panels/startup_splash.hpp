#ifndef RR_EDITOR_PANELS_STARTUP_SPLASH_H
#define RR_EDITOR_PANELS_STARTUP_SPLASH_H

#include "rurouni/editor/state.hpp"

#include "rurouni/core/module.hpp"
#include "rurouni/event/event_system.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/types/uuid.hpp"

#include <functional>
#include <optional>

namespace rr::editor::ui {

class StartupSplash {
   public:
    static void draw(UIState& state,
                     event::EventSystem& eventSystem,
                     std::unordered_map<UUID, ModuleHistoryItem>& history,
                     std::function<void(const system::Path&,
                                        const std::string& name)> importFunc,
                     std::function<void(const UUID&)> openFunc);

   private:
    static void draw_history(
        std::unordered_map<UUID, ModuleHistoryItem>& history,
        UUID& selectedId);
    static void draw_import(
        std::function<void(const system::Path&, const std::string& name)>
            importFunc);
    static void draw_exit(event::EventSystem& eventSystem);
};

}  // namespace rr::editor::ui

#endif  // !RR_EDITOR_PANELS_STARTUP_SPLASH_H
