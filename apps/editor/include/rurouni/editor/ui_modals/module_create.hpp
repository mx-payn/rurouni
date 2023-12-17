#ifndef RR_EDITOR_UI_MODALS_MODULE_CREATE_H
#define RR_EDITOR_UI_MODALS_MODULE_CREATE_H

#include "rurouni/editor/state.hpp"

namespace rr::editor::ui {

class ModuleCreateModal {
   public:
    ModuleCreateModal() = default;

    void draw(UIState& state,
              std::function<void(const system::Path&, const std::string&)>
                  createFunc);
};

}  // namespace rr::editor::ui

#endif  // !RR_EDITOR_UI_MODALS_MODULE_CREATE_H
