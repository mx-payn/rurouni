#ifndef RR_RUNTIME_H
#define RR_RUNTIME_H

// rurouni
#include "rurouni/event/event.hpp"
#include "rurouni/event/event_system.hpp"
#include "rurouni/event/window_event.hpp"
#include "rurouni/graphics/window.hpp"
#include "rurouni/system/filesystem.hpp"

// std
#include <memory>

namespace rr {

class Runtime : public event::Subscriber {
   public:
    Runtime(const graphics::WindowSpecification& windowSpec);
    ~Runtime();

    void run();

    virtual void on_event(std::shared_ptr<event::Event> event) override;

   private:
    void update(float dt);
    void render();

    void on_window_close_event(std::shared_ptr<event::WindowClose> event);

   private:
    bool m_Running;
    float m_DeltaTime;

    std::shared_ptr<graphics::Window> m_Window;
    std::shared_ptr<event::EventSystem> m_EventSystem;

    std::string m_AppName;
    system::Path m_ExecPath;
    system::Path m_SharedDataDir;
    system::Path m_SharedConfigDir;
    system::Path m_UserDataDir;
    system::Path m_UserConfigDir;
};

}  // namespace rr

#endif  // !RR_RUNTIME_H
