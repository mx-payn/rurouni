#ifndef RR_LIBS_CORE_PROJECT_H
#define RR_LIBS_CORE_PROJECT_H

#include "rurouni/system/filesystem.hpp"
#include "rurouni/types/uuid.hpp"

#include <cereal/cereal.hpp>

namespace rr::core {

struct Project {
    std::string Name;
    UUID Id;

    system::Path ProjectPath;     //!< absolte path to the project dir
    system::Path StartScenePath;  //!< relative path to start scene
};

template <class Archive>
void serialize(Archive& archive, Project& p) {
    archive(cereal::make_nvp("name", p.Name), cereal::make_nvp("id", p.Id),
            cereal::make_nvp("project_path", p.ProjectPath),
            cereal::make_nvp("start_scene_path", p.StartScenePath));
}

}  // namespace rr::core

#endif  // !RR_LIBS_CORE_PROJECT_H
