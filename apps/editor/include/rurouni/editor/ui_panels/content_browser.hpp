#ifndef RR_EDITOR_UI_PANELS_CONTENT_BROWSER_H
#define RR_EDITOR_UI_PANELS_CONTENT_BROWSER_H

#include "rurouni/editor/state.hpp"

#include "rurouni/system/filesystem.hpp"

namespace rr::editor::ui {

class ContentBrowser {
   public:
    static void draw(UIState& state, const system::Path& rootDir);
};

}  // namespace rr::editor::ui

#endif  // !RR_EDITOR_UI_PANELS_CONTENT_BROWSER_H
