#ifndef RR_LIBS_CORE_MODULE_H
#define RR_LIBS_CORE_MODULE_H

#include "rurouni/core/scene.hpp"
#include "rurouni/error.hpp"
#include "rurouni/event/event_system.hpp"
#include "rurouni/graphics/batch_renderer.hpp"
#include "rurouni/graphics/window.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/types/uuid.hpp"

#include "fmt/core.h"

#include <memory>

namespace rr::core {

class Module {
   public:
    Module(std::shared_ptr<graphics::Window> window,
           std::shared_ptr<graphics::BatchRenderer> renderer,
           std::shared_ptr<event::EventSystem> eventSystem);
    ~Module();

    std::optional<Error> load_from_file(const system::Path& path);
    std::optional<Error> write_to_file(const system::Path& path);

    void on_update(float dt);
    void on_render();

    const std::string& get_name() const { return m_Name; }
    void set_name(const std::string& name) { m_Name = name; }
    const UUID& get_id() const { return m_Id; }
    void set_id(const UUID& id) { m_Id = id; }
    const system::Path& get_root_path() const { return m_RootPath; }
    const system::Path& get_start_scene_path() const {
        return m_StartScenePath;
    }
    void set_start_scene_path(const system::Path& path) {
        m_StartScenePath = path;
    }

    std::unique_ptr<Scene>& get_current_scene() { return m_CurrentScene; }

   private:
    std::string m_Name;
    UUID m_Id;

    std::unique_ptr<Scene> m_CurrentScene;
    std::shared_ptr<graphics::Window> m_Window;
    std::shared_ptr<graphics::BatchRenderer> m_Renderer;
    std::shared_ptr<event::EventSystem> m_EventSystem;

    system::Path m_RootPath;
    system::Path m_StartScenePath;  //!< relative path to start scene
};

}  // namespace rr::core

#endif  // !RR_LIBS_CORE_PROJECT_H
