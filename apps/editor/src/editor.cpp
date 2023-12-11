// editor
#include "rurouni/editor.hpp"
#include <functional>
#include <memory>
#include "rurouni/editor/config.h"
#include "rurouni/editor/logger.hpp"

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

namespace rr {

Editor::Editor(const graphics::WindowSpecification& windowSpec) {
    // initializing loggers
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_st>());
    sinks.push_back(
        std::make_shared<spdlog::sinks::basic_file_sink_st>("out.log"));

    graphics::init_logger(sinks);
    common::init_logger(sinks);
    editor::init_logger(sinks);

    spdlog::set_level(spdlog::level::level_enum::trace);
    spdlog::flush_on(spdlog::level::level_enum::trace);

    // filepath info
    editor::info("bin dir ......... : {}", RR_EDITOR_BIN_DIR);
    editor::info("lib dir ......... : {}", RR_EDITOR_LIB_DIR);
    editor::info("archive dir ..... : {}", RR_EDITOR_ARCHIVE_DIR);
    editor::info("shared data dir . : {}", RR_EDITOR_SHARED_DATA_DIR);
    editor::info("shared config dir : {}", RR_EDITOR_SHARED_CONFIG_DIR);
    editor::info("user data dir ... : {}", RR_EDITOR_USER_DATA_DIR);
    editor::info("user config dir . : {}", RR_EDITOR_USER_CONFIG_DIR);

    m_EventSystem = std::make_shared<event::EventSystem>();
    m_EventSystem->subscribe<event::WindowClose>(this);

    m_Window = std::make_shared<graphics::Window>(windowSpec, m_EventSystem);
}

Editor::~Editor() {
    m_EventSystem->unsubscribe_from_all(this);
}

void Editor::run() {
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

void Editor::update(float dt) {
    m_Window->update(dt);
}

void Editor::render() {
    graphics::api::set_clear_color({0.05f, 0.05f, 0.05f, 1.0f});
    graphics::api::clear();

    m_Window->swap_buffers();
}

void Editor::on_event(std::shared_ptr<event::Event> event) {
    event::dispatch<event::WindowClose>(
        event,
        std::bind(&Editor::on_window_close_event, this, std::placeholders::_1));
}

void Editor::on_window_close_event(std::shared_ptr<event::WindowClose> event) {
    editor::trace("received window close event. closing loop...");
    m_Running = false;
}

}  // namespace rr
