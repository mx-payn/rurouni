#ifndef RR_EDITOR_STATE_H
#define RR_EDITOR_STATE_H

#include "rurouni/math/vec.hpp"
#include "rurouni/system/filesystem.hpp"

#include <string>

namespace rr::editor {

struct ModuleHistoryItem {
    std::string Name;
    system::Path RootPath;
};

struct UIState {
    math::ivec2 SceneViewportSize = math::ivec2(1.0f);

    bool ShowModuleCreateModal = false;
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
