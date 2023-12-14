#ifndef RR_SANDBOX_LOGGER_H
#define RR_SANDBOX_LOGGER_H

#include "rurouni/system/debug_break.hpp"

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <vector>
#include <experimental/source_location>

namespace rr::sandbox {

    static std::string logger_name = "sandbox";

    static void init_logger(std::vector<spdlog::sink_ptr> sinks) {
        auto logger = spdlog::get(logger_name);
        if(!logger)
        {
            if(sinks.size() > 0)
            {
                logger = std::make_shared<spdlog::logger>(logger_name,
                                                          std::begin(sinks),
                                                          std::end(sinks));
                spdlog::register_logger(logger);
            }
            else
            {
                logger = std::make_shared<spdlog::logger>(logger_name);
                spdlog::register_logger(logger);
            }
        }
    }

    /** logs message with trace level. */
    template <class... Args>
    void trace(fmt::format_string<Args...> fmt, Args&&... args) {
        spdlog::get(logger_name)->trace(fmt, std::forward<Args>(args)...);
    }
    /** logs message with debug level. */
    template <class... Args>
    void debug(fmt::format_string<Args...> fmt, Args&&... args) {
        spdlog::get(logger_name)->debug(fmt, std::forward<Args>(args)...);
    }
    /** logs message with info level. */
    template <class... Args>
    void info(fmt::format_string<Args...> fmt, Args&&... args) {
        spdlog::get(logger_name)->info(fmt, std::forward<Args>(args)...);
    }
    /** logs message with warn level. */
    template <class... Args>
    void warn(fmt::format_string<Args...> fmt, Args&&... args) {
        spdlog::get(logger_name)->warn(fmt, std::forward<Args>(args)...);
    }
    /** logs message with error level. */
    template <class... Args>
    void error(fmt::format_string<Args...> fmt, Args&&... args) {
        spdlog::get(logger_name)->error(fmt, std::forward<Args>(args)...);
    }
    /** logs message with critical level. */
    template <class... Args>
    void critical(fmt::format_string<Args...> fmt, Args&&... args) {
        spdlog::get(logger_name)->critical(fmt, std::forward<Args>(args)...);
    }

    /** debug breaks and prints location on assertion failure */
    template <typename T>
    void require(T&& assertion,
                 const std::string_view msg,
                 const std::experimental::source_location location =
                     std::experimental::source_location::current()) {
        if (!assertion) {
            spdlog::get(logger_name)->critical("{}:{} -> {} *** assertion failed",
                              location.file_name(), location.line(),
                              location.function_name());
            spdlog::get(logger_name)->critical("{}", msg);

            RR_DEBUG_BREAK;
        }
    }

}

#endif  // !RR_SANDBOX_LOGGER_H
