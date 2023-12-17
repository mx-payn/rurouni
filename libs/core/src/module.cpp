#include "rurouni/pch.hpp"

#include "rurouni/core/logger.hpp"
#include "rurouni/core/module.hpp"
#include "rurouni/system/filesystem.hpp"

#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>

namespace rr::core {

Module::Module(const system::Path& path,
               std::shared_ptr<graphics::Window> window,
               std::shared_ptr<graphics::BatchRenderer> renderer,
               std::shared_ptr<event::EventSystem> eventSystem)
    : m_ModulePath(path),
      m_Window(window),
      m_Renderer(renderer),
      m_EventSystem(eventSystem) {}

Module::~Module() {}

std::optional<Error> Module::load_from_file() {
    return load_from_file(m_ModulePath);
}

std::optional<Error> Module::load_from_file(const system::Path& path) {
    if (!system::exists(path)) {
        error("the provided path does not exist on the filesystem. path: {}",
              path);
        return Error(
            "the provided path does not exist on the filesystem. path: {}",
            path);
    }
    if (!system::is_regular_file(path)) {
        error("the provided path does not lead to a regular file. path: {}",
              path);
        return Error(
            "the provided path does not lead to a regular file. path: {}",
            path);
    }

    try {
        std::ifstream is(path);
        {
            cereal::JSONInputArchive input(is);
            input(cereal::make_nvp("name", m_Name),
                  cereal::make_nvp("id", m_Id),
                  cereal::make_nvp("module_path", m_ModulePath),
                  cereal::make_nvp("start_scene_path", m_StartScenePath));
        }
        is.close();
    } catch (std::exception e) {
        error("error while loading module from path: {}", path);
        error("{}", e.what());
        return Error("error while loading module from path: {}, message: {}",
                     e.what());
    }

    return {};
}

std::optional<Error> Module::write_to_file() {
    return write_to_file(m_ModulePath);
}

std::optional<Error> Module::write_to_file(const system::Path& path) {
    if (system::exists(path)) {
        warn(
            "the provided path already exists on the filesystem and is "
            "overwritten. path: {}",
            path);

        if (!system::is_regular_file(path)) {
            error(
                "the provided path already exists and does not lead to a "
                "regular file. path: {}",
                path);
            return Error(
                "the provided path already exists does not lead to a regular "
                "file. path: {}",
                path);
        }
    }

    try {
        std::ofstream os(path);
        {
            cereal::JSONOutputArchive out(os);
            out(cereal::make_nvp("name", m_Name), cereal::make_nvp("id", m_Id),
                cereal::make_nvp("module_path", m_ModulePath),
                cereal::make_nvp("start_scene_path", m_StartScenePath));
        }
        os.close();
    } catch (std::exception e) {
        error("error while writing module to path: {}", path);
        error("{}", e.what());
        return Error("error while writing module to path: {}, message: {}",
                     e.what());
    }

    return {};
}

void Module::on_update(float dt) {
    if (m_CurrentScene != nullptr) {
        m_CurrentScene->on_update(dt);
    }
}

void Module::on_render() {
    if (m_CurrentScene != nullptr) {
        m_CurrentScene->on_render(*m_Renderer);
    }
}
}  // namespace rr::core
