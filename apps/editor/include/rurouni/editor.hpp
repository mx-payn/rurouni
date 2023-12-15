#ifndef RR_EDITOR_H
#define RR_EDITOR_H

// editor
#include "rurouni/editor/ui.hpp"

// rurouni
#include "rurouni/event/event.hpp"
#include "rurouni/event/event_system.hpp"
#include "rurouni/event/window_event.hpp"
#include "rurouni/graphics/window.hpp"
#include "rurouni/system/filesystem.hpp"

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

   private:
    bool m_Running;
    float m_DeltaTime;

    ui::State m_UIState;

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
