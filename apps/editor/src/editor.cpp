// editor
#include "rurouni/editor.hpp"
#include "rurouni/editor/config.h"
#include "rurouni/editor/logger.hpp"
#include "rurouni/editor/state.hpp"
#include "rurouni/editor/ui.hpp"
#include "rurouni/editor/ui_panels/startup_splash.hpp"

// rurouni
#include "rurouni/common/logger.hpp"
#include "rurouni/event/application_event.hpp"
#include "rurouni/event/event_system.hpp"
#include "rurouni/event/window_event.hpp"
#include "rurouni/graphics/logger.hpp"
#include "rurouni/graphics/render_api.hpp"
#include "rurouni/graphics/window.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/time.hpp"

// external
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/unordered_map.hpp>

// std
#include <fstream>
#include <functional>
#include <memory>

namespace rr::editor {

Editor::Editor(const graphics::WindowSpecification& windowSpec) {
    m_ExecPath = system::get_current_executable_path();
    m_AppName = m_ExecPath.stem();
    m_SharedDataDir = m_ExecPath.parent_path() / RR_EDITOR_RELATIVE_DATA_DIR;
    m_SharedConfigDir =
        m_ExecPath.parent_path() / RR_EDITOR_RELATIVE_CONFIG_DIR;
    m_UserDataDir = system::get_app_user_data_dir(m_AppName);
    m_UserConfigDir = system::get_app_user_config_dir(m_AppName);

    // initializing loggers
    std::string currentDateTime = time::get_current_date_and_time();
    system::Path logPath =
        m_UserDataDir / "logs" / currentDateTime.append(".log");
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_st>());
    sinks.push_back(
        std::make_shared<spdlog::sinks::basic_file_sink_st>(logPath));

    graphics::init_logger(sinks);
    common::init_logger(sinks);
    editor::init_logger(sinks);

    spdlog::set_level(spdlog::level::level_enum::trace);
    spdlog::flush_on(spdlog::level::level_enum::trace);

    // filepath info
    editor::info("app name ........ : {}", m_AppName);
    editor::info("exec path ....... : {}", m_ExecPath);
    editor::info("exec dir ........ : {}", m_ExecPath.parent_path());
    editor::info("shared data dir . : {}", m_SharedDataDir);
    editor::info("shared config dir : {}", m_SharedConfigDir);
    editor::info("user data dir ... : {}", m_UserDataDir);
    editor::info("user config dir . : {}", m_UserConfigDir);

    m_EventSystem = std::make_shared<event::EventSystem>();
    m_EventSystem->subscribe<event::WindowClose>(this);
    m_EventSystem->subscribe<event::ApplicationClose>(this);

    m_Window = std::make_shared<graphics::Window>(windowSpec, m_EventSystem);

    ui::init(*m_Window, m_SharedDataDir, m_UserDataDir, m_UserConfigDir);

    // read project history
    system::Path historyPath = m_UserConfigDir / "project_history.json";

    if (system::exists(historyPath)) {
        try {
            std::ifstream is(historyPath);
            cereal::JSONInputArchive in(is);
            in(m_ModuleHistory);
        } catch (cereal::Exception& e) {
            error("cereal: {}", e.what());
            error("path: {}", historyPath.string());

            // ui::push_error(
            //     "project history",
            //     fmt::format("error while reading {}", historyPath.string()),
            //     e.what());
        }
    }
}

Editor::~Editor() {
    m_EventSystem->unsubscribe_from_all(this);
    ui::terminate();
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
    ui::update(dt, m_UserConfigDir);
}

void Editor::render() {
    graphics::api::set_clear_color({0.05f, 0.05f, 0.05f, 1.0f});
    graphics::api::clear();
    ui::begin();

    if (m_CurrentModule.has_value()) {
        ui::draw_dockspace(m_UIState);
    } else {
        ui::StartupSplash::draw(
            m_UIState, *m_EventSystem, m_ModuleHistory,
            std::bind(&Editor::import_module, this, std::placeholders::_1,
                      std::placeholders::_2),
            std::bind(&Editor::open_module, this, std::placeholders::_1));
    }

    ui::end();
    m_Window->swap_buffers();
}

void Editor::on_event(std::shared_ptr<event::Event> event) {
    event::dispatch<event::WindowClose>(
        event,
        std::bind(&Editor::on_window_close_event, this, std::placeholders::_1));
    event::dispatch<event::ApplicationClose>(
        event, std::bind(&Editor::on_application_close_event, this,
                         std::placeholders::_1));
}

void Editor::on_window_close_event(std::shared_ptr<event::WindowClose> event) {
    m_Running = false;
}

void Editor::on_application_close_event(
    std::shared_ptr<event::ApplicationClose> event) {
    m_Running = false;
}

void Editor::create_module(const system::Path& path, const std::string& name) {}
void Editor::import_module(const system::Path& path, const std::string& name) {}
void Editor::open_module(const UUID& id) {}

}  // namespace rr::editor
