// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/dev/logger.hpp"
#include "rurouni/system/filesystem.hpp"

// external
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace rr::dev {

static bool s_Initialized = false;

// static state initializations
std::vector<spdlog::sink_ptr> Logger::s_LogSinks =
    std::vector<spdlog::sink_ptr>();
LogLevel Logger::s_LogLevel = LogLevel::Info;
LogLevel Logger::s_FlushLevel = LogLevel::Error;

void Logger::init(const LoggerSpecification& spec) {
    // only init once
    if (s_Initialized) {
        return;
    }

    // file sink
    if (!spec.LogfilePath.empty()) {
        // create parent dir if it doesn't exist
        if (!system::exists(spec.LogfilePath.parent_path())) {
            system::create_directories(spec.LogfilePath.parent_path());
        }

        auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            spec.LogfilePath, true);
        fileSink->set_pattern("[%T][%=8l] %n: %v");
        s_LogSinks.emplace_back(fileSink);
    }

    // stdout sink
    if (spec.LogStdOut) {
        auto stdoutSink =
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        stdoutSink->set_pattern("[%T][%^%=8l%$] %n: %v");
        s_LogSinks.emplace_back(stdoutSink);
    }

    // saving some static state
    // other loggers created will use the static state
    // initially, but may set their own levels after construction
    s_LogLevel = spec.Level;
    s_FlushLevel = spec.FlushLevel;
    s_Initialized = true;
}

Logger::Logger(const std::string& name)
    : m_Logger(name, s_LogSinks.begin(), s_LogSinks.end()) {
    m_Logger.set_level(static_cast<spdlog::level::level_enum>(s_LogLevel));
    m_Logger.flush_on(static_cast<spdlog::level::level_enum>(s_FlushLevel));
}

void Logger::set_log_level(LogLevel level) {
    m_Logger.set_level(static_cast<spdlog::level::level_enum>(level));
}

void Logger::set_flush_level(LogLevel level) {
    m_Logger.flush_on(static_cast<spdlog::level::level_enum>(level));
}

}  // namespace rr::dev
