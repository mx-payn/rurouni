// sandbox
#include "rurouni/graphics/shader.hpp"
#include "rurouni/sandbox/config.h"
#include "rurouni/sandbox.hpp"
#include "rurouni/sandbox/logger.hpp"

// rurouni
#include "rurouni/common/logger.hpp"
#include "rurouni/core/logger.hpp"
#include "rurouni/core/scene.hpp"
#include "rurouni/event/event_system.hpp"
#include "rurouni/event/window_event.hpp"
#include "rurouni/graphics/batch_renderer.hpp"
#include "rurouni/graphics/default/shader.hpp"
#include "rurouni/graphics/logger.hpp"
#include "rurouni/graphics/render_api.hpp"
#include "rurouni/graphics/window.hpp"
#include "rurouni/system/command.hpp"
#include "rurouni/system/filesystem.hpp"

// external
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

// std
#include <cmath>
#include <memory>
#include <string>

namespace rr {

    Sandbox::Sandbox(const graphics::WindowSpecification& windowSpec
            ) {
        // initializing loggers
        std::vector<spdlog::sink_ptr> sinks;
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_st>());

        rr::graphics::init_logger(sinks);
        rr::common::init_logger(sinks);
        rr::core::init_logger(sinks);
        rr::sandbox::init_logger(sinks);

        spdlog::set_level(spdlog::level::level_enum::trace);
        spdlog::flush_on(spdlog::level::level_enum::trace);

        system::Path execPath =
            system::get_current_executable_path().parent_path();
        system::Path dataDir = execPath / RR_RELATIVE_SANDBOX_DATA_DIR;
        system::Path configDir = execPath / RR_RELATIVE_SANDBOX_CONFIG_DIR;
        system::Path userDataDir =
            system::get_app_user_data_dir(RR_SANDBOX_APP_NAME);
        system::Path userConfigDir =
            system::get_app_user_config_dir(RR_SANDBOX_APP_NAME);

        // filepath info
        sandbox::info("exec dir : {}", execPath);
        sandbox::info("data dir : {}", dataDir);
        sandbox::info("config dir : {}", configDir);
        sandbox::info("user data dir : {}", userDataDir);
        sandbox::info("user config dir : {}", userConfigDir);

        m_EventSystem = std::make_shared<event::EventSystem>();
        m_EventSystem->subscribe<event::WindowClose>(this);
        m_EventSystem->subscribe<event::WindowFramebufferResize>(this);
        m_Window = std::make_shared<graphics::Window>(windowSpec, m_EventSystem);
        m_Renderer = std::make_shared<graphics::BatchRenderer>();
        m_CurrentScene = std::make_shared<core::Scene>(
                m_Window->get_window_data().FramebufferSize
                );
        m_CurrentScene->load_scene(system::Path(dataDir) / "scene.json",
                                   m_Window->get_window_data().FramebufferSize);
        m_PostFXShader = std::make_unique<graphics::Shader>(
                graphics::DEFAULT_POST_FX_SHADER_VERTEX_SRC,
                graphics::DEFAULT_POST_FX_SHADER_FRAGMENT_SRC
                );
    }

    Sandbox::~Sandbox() {
        m_EventSystem->unsubscribe_from_all(this);
    }

    void Sandbox::run() {
        std::chrono::high_resolution_clock timer;
        auto last_update = timer.now();

        m_Running = true;
        while(m_Running) {
            auto now = timer.now();
            m_DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(
                              now - last_update)
                              .count() /
                          1000.0f;
            last_update = now;

            update(m_DeltaTime);
            render();
        }
    }

    void Sandbox::update(float dt) {
        m_Window->update(dt);
        m_CurrentScene->on_update(dt);
    }

    void Sandbox::render() {
        graphics::api::set_clear_color({0.0f, 0.0f, 0.0f, 0.0f});
        graphics::api::clear();

        m_CurrentScene->on_render(*m_Renderer);

        // draw scene to default framebuffer
        graphics::api::set_viewport(
            m_Window->get_window_data().FramebufferSize);
        m_PostFXShader->bind();
        m_CurrentScene->get_framebuffer().draw_color_attachement(0);
        m_PostFXShader->unbind();

        m_Window->swap_buffers();
    }

    void Sandbox::on_event(std::shared_ptr<event::Event> event) {
        sandbox::trace("received event: {}", event->to_string());
        event::dispatch<event::WindowClose>(event, std::bind(&Sandbox::on_window_close_event, this, std::placeholders::_1));
        event::dispatch<event::WindowFramebufferResize>(event, std::bind(&Sandbox::on_frambuffer_resize_event, this, std::placeholders::_1));
    }

    void Sandbox::on_window_close_event(std::shared_ptr<event::WindowClose> event) {
        m_Running = false;
    }

    void Sandbox::on_frambuffer_resize_event(std::shared_ptr<event::WindowFramebufferResize> event) {
        m_CurrentScene->set_viewport_size(event->get_new_size());
    }
}
