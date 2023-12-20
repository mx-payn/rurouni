#ifndef RR_EDITOR_H
#define RR_EDITOR_H

// rurouni
#include "rurouni/core/asset_manager.hpp"
#include "rurouni/core/scene.hpp"
#include "rurouni/event/event.hpp"
#include "rurouni/event/event_system.hpp"
#include "rurouni/event/window_event.hpp"
#include "rurouni/graphics/batch_renderer.hpp"
#include "rurouni/graphics/font.hpp"
#include "rurouni/graphics/shader.hpp"
#include "rurouni/graphics/text.hpp"
#include "rurouni/graphics/window.hpp"

// std
#include <memory>

namespace rr {

class Sandbox : public event::Subscriber {
    public:
        Sandbox(const graphics::WindowSpecification& windowSpec
                );
        ~Sandbox();

        void run();

        virtual void on_event(std::shared_ptr<event::Event> event) override;

    private:
        void update(float dt);
        void render();

        void on_window_close_event(std::shared_ptr<event::WindowClose> event);
        void on_frambuffer_resize_event(std::shared_ptr<event::WindowFramebufferResize> event);

    private:
        bool m_Running;
        float m_DeltaTime;

        std::shared_ptr<graphics::Window> m_Window;
        std::shared_ptr<core::AssetManager> m_AssetManager;
        std::shared_ptr<event::EventSystem> m_EventSystem;
        std::shared_ptr<graphics::BatchRenderer> m_Renderer;
        std::shared_ptr<core::Scene> m_CurrentScene;

        std::shared_ptr<graphics::Shader> m_PostFXShader;
};

}

#endif // !RR_EDITOR_H
