#ifndef RR_EDITOR_H
#define RR_EDITOR_H

// rurouni
#include "rurouni/event/event.hpp"
#include "rurouni/event/event_system.hpp"
#include "rurouni/event/window_event.hpp"
#include "rurouni/graphics/window.hpp"

// std
#include <memory>

namespace rr {

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

   private:
    bool m_Running;
    float m_DeltaTime;

    std::shared_ptr<graphics::Window> m_Window;
    std::shared_ptr<event::EventSystem> m_EventSystem;
};

}  // namespace rr

#endif  // !RR_EDITOR_H
