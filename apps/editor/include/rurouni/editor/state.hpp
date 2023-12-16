#ifndef RR_EDITOR_STATE_H
#define RR_EDITOR_STATE_H

#include "rurouni/system/filesystem.hpp"

#include <string>

namespace rr::editor {

struct ProjectHistoryItem {
    std::string Name;
    system::Path Path;
};

struct UIState {
    bool ShowModalCreateProject = false;
};

}

#endif // !RR_EDITOR_STATE_H
