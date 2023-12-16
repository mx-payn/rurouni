#ifndef RR_EDITOR_H
#define RR_EDITOR_H

// editor
#include "rurouni/core/scene.hpp"
#include "rurouni/editor/ui.hpp"

// rurouni
#include "rurouni/event/application_event.hpp"
#include "rurouni/event/event.hpp"
#include "rurouni/event/event_system.hpp"
#include "rurouni/event/window_event.hpp"
#include "rurouni/graphics/window.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/core/project.hpp"

// std
#include <memory>

namespace rr::editor {

class Editor : public event::Subscriber {
   public:
    Editor(const graphics::WindowSpecification& windowSpec);
    ~Editor();

    void run();

    virtual void on_event(std::shared_ptr<event::Event> event) override;

   private:
    void update(float dt);
    void render();

    void on_window_close_event(std::shared_ptr<event::WindowClose> event);
    void on_window_framebuffer_resize(
        std::shared_ptr<event::WindowFramebufferResize> event);
    void on_application_close_event(std::shared_ptr<event::ApplicationClose> event);

   private:
    void create_project(const system::Path& path, const std::string& name);
    void import_project(const system::Path& path, const std::string& name);
    void open_project(const UUID& id);

   private:
    // state
    bool m_Running;
    float m_DeltaTime;
    UIState m_UIState;
    std::optional<core::Project> m_CurrentProject;
    std::unique_ptr<core::Scene> m_CurrentScene;
    std::unordered_map<UUID, ProjectHistoryItem> m_ProjectHistory;

    // systems
    std::shared_ptr<graphics::Window> m_Window;
    std::shared_ptr<event::EventSystem> m_EventSystem;

    // paths
    std::string m_AppName;
    system::Path m_ExecPath;
    system::Path m_SharedDataDir;
    system::Path m_SharedConfigDir;
    system::Path m_UserDataDir;
    system::Path m_UserConfigDir;
};

}  // namespace rr::editor

#endif  // !RR_EDITOR_H
