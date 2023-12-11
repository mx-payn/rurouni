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
    runtime::info("bin dir ......... : {}", RR_RUNTIME_BIN_DIR);
    runtime::info("lib dir ......... : {}", RR_RUNTIME_LIB_DIR);
    runtime::info("archive dir ..... : {}", RR_RUNTIME_ARCHIVE_DIR);
    runtime::info("shared data dir . : {}", RR_RUNTIME_SHARED_DATA_DIR);
    runtime::info("shared config dir : {}", RR_RUNTIME_SHARED_CONFIG_DIR);
    runtime::info("user data dir ... : {}", RR_RUNTIME_USER_DATA_DIR);
    runtime::info("user config dir . : {}", RR_RUNTIME_USER_CONFIG_DIR);

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
