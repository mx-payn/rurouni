#ifndef RR_EDITOR_STATE_H
#define RR_EDITOR_STATE_H

#include "rurouni/graphics/texture.hpp"
#include "rurouni/math/vec.hpp"
#include "rurouni/system/filesystem.hpp"

#include <entt/entt.hpp>

#include <optional>
#include <string>

namespace rr::editor {

struct ModuleHistoryItem {
    std::string Name;
    system::Path RootPath;
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
