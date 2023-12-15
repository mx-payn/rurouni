// editor
#include "rurouni/runtime.hpp"
#include "rurouni/runtime/config.h"
#include "rurouni/runtime/logger.hpp"

// rurouni
#include "rurouni/common/logger.hpp"
#include "rurouni/event/event_system.hpp"
#include "rurouni/event/window_event.hpp"
#include "rurouni/graphics/logger.hpp"
#include "rurouni/graphics/render_api.hpp"
#include "rurouni/graphics/window.hpp"

// external
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

// std
#include <functional>
#include <memory>

namespace rr {

Runtime::Runtime(const graphics::WindowSpecification& windowSpec) {
    // paths
    m_AppName = RR_RUNTIME_APP_NAME;
    m_ExecPath = system::get_current_executable_path();
    m_SharedDataDir = m_ExecPath.parent_path() / RR_RUNTIME_RELATIVE_DATA_DIR;
    m_SharedConfigDir =
        m_ExecPath.parent_path() / RR_RUNTIME_RELATIVE_CONFIG_DIR;
    m_UserDataDir = system::get_app_user_data_dir(RR_RUNTIME_APP_NAME);
    m_UserConfigDir = system::get_app_user_config_dir(RR_RUNTIME_APP_NAME);

    // initializing loggers
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_st>());
    sinks.push_back(
        std::make_shared<spdlog::sinks::basic_file_sink_st>("out.log"));

    rr::graphics::init_logger(sinks);
    rr::common::init_logger(sinks);
    rr::runtime::init_logger(sinks);

    spdlog::set_level(spdlog::level::level_enum::trace);
    spdlog::flush_on(spdlog::level::level_enum::trace);

    // filepath info
    runtime::info("app name ........ : {}", m_AppName);
    runtime::info("exec path ....... : {}", m_ExecPath);
    runtime::info("exec dir ........ : {}", m_ExecPath.parent_path());
    runtime::info("shared data dir . : {}", m_SharedDataDir);
    runtime::info("shared config dir : {}", m_SharedConfigDir);
    runtime::info("user data dir ... : {}", m_UserDataDir);
    runtime::info("user config dir . : {}", m_UserConfigDir);

    m_EventSystem = std::make_shared<event::EventSystem>();
    m_EventSystem->subscribe<event::WindowClose>(this);

    m_Window = std::make_shared<graphics::Window>(windowSpec, m_EventSystem);
}

Runtime::~Runtime() {
    m_EventSystem->unsubscribe_from_all(this);
}

void Runtime::run() {
    std::chrono::high_resolution_clock timer;
    auto last_update = timer.now();

    m_Running = true;
    while (m_Running) {
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

void Runtime::update(float dt) {
    m_Window->update(dt);
}

void Runtime::render() {
    graphics::api::set_clear_color({0.05f, 0.05f, 0.05f, 1.0f});
    graphics::api::clear();

    m_Window->swap_buffers();
}

void Runtime::on_event(std::shared_ptr<event::Event> event) {
    event::dispatch<event::WindowClose>(
        event, std::bind(&Runtime::on_window_close_event, this,
                         std::placeholders::_1));
}

void Runtime::on_window_close_event(std::shared_ptr<event::WindowClose> event) {
    runtime::trace("received window close event. closing loop...");
    m_Running = false;
}

}  // namespace rr
