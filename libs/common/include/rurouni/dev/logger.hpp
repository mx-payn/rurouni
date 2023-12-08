#ifndef RR_LIBS_COMMON_DEV_LOGGER_H
#define RR_LIBS_COMMON_DEV_LOGGER_H

// rurouni
#include "rurouni/system/filesystem.hpp"

// external
#include <fmt/core.h>
#include <spdlog/spdlog.h>

// std
#include <experimental/source_location>
#include <memory>
#include <string>
#include <vector>

/** Cross platform macros for debug breaking. */
#if (defined(__debugbreak) || (_MSC_VER))
#define RR_DEBUG_BREAK __debugbreak()
#elif (defined(__breakpoint) || (__ARMCC_VERSION))
#define RR_DEBUGBREAK __breakpoint(42)
#else
#include <signal.h>
#if (defined(SIGTRAP))
#define RR_DEBUG_BREAK raise(SIGTRAP)
#else
#define RR_DEBUG_BREAK raise(SIGABRT)
#endif
#endif

namespace rr::dev {

/** your typical log levels. */
enum class LogLevel { Trace, Debug, Info, Warn, Error, Critical, Off };

/** logging behaviour variables.
 *
 *  only used once when initializing the logger class. log sinks
 *  and logger instances get initialized according to the initially
 *  passed LoggerSpecification.
 */
struct LoggerSpecification {
    system::Path LogfilePath;               //!< if empty, no file logging
    LogLevel Level = LogLevel::Info;        //!< respected messages
    LogLevel FlushLevel = LogLevel::Error;  //!< flush on message level
    bool LogStdOut = false;                 //!< add a sink to stdout
};

/** a class constructing logger objects.
 *
 *  soft wrapper around spdlog (public spdlog link).
 *  the idea is that the default state is initialized once and all instances
 *  create their own spdlog::logger, but use the same sinks. results in only
 *  one file with all log messages from libs and clients.
 *
 *  NOTE(max): don't know yet, whether this is smart or just overengineered.
 *             at the end it should just write stuff...
 */
class Logger {
   public:
    /** constructs a new logger object based on the static state.
     *
     *  \param name the logger name displayed in each message.
     */
    Logger(const std::string& name);
    ~Logger() = default;

    /** initializes the static construction state.
     *
     *  pushes log_sinks and sets levels according to the given
     *  LoggerSpecification to the static construction state.
     *
     *  \param spec static state specification.
     */
    static void init(const LoggerSpecification& spec);

    /** logs message with trace level. */
    template <class... Args>
    void trace(fmt::format_string<Args...> fmt, Args&&... args) {
        m_Logger.trace(fmt, std::forward<Args>(args)...);
    }
    /** logs message with debug level. */
    template <class... Args>
    void debug(fmt::format_string<Args...> fmt, Args&&... args) {
        m_Logger.debug(fmt, std::forward<Args>(args)...);
    }
    /** logs message with info level. */
    template <class... Args>
    void info(fmt::format_string<Args...> fmt, Args&&... args) {
        m_Logger.info(fmt, std::forward<Args>(args)...);
    }
    /** logs message with warn level. */
    template <class... Args>
    void warn(fmt::format_string<Args...> fmt, Args&&... args) {
        m_Logger.warn(fmt, std::forward<Args>(args)...);
    }
    /** logs message with error level. */
    template <class... Args>
    void error(fmt::format_string<Args...> fmt, Args&&... args) {
        m_Logger.error(fmt, std::forward<Args>(args)...);
    }
    /** logs message with critical level. */
    template <class... Args>
    void critical(fmt::format_string<Args...> fmt, Args&&... args) {
        m_Logger.critical(fmt, std::forward<Args>(args)...);
    }

    /** debug breaks and prints location on assertion failure */
    template <typename T>
    void require(T&& assertion,
                 const std::string_view msg,
                 const std::experimental::source_location location =
                     std::experimental::source_location::current()) {
        if (!assertion) {
            m_Logger.critical("{}:{} -> {} *** assertion failed",
                              location.file_name(), location.line(),
                              location.function_name());
            m_Logger.critical("{}", msg);

            RR_DEBUG_BREAK;
        }
    }

    /** sets the instances level threshold. */
    void set_log_level(LogLevel level);
    /** sets the instances level to flush stored messages to sinks. */
    void set_flush_level(LogLevel level);

   private:
    spdlog::logger m_Logger;

    // construction state
    static std::vector<spdlog::sink_ptr> s_LogSinks;
    static LogLevel s_LogLevel;
    static LogLevel s_FlushLevel;
};

}  // namespace rr::dev

#endif  // !RR_LIBS_COMMON_DEV_LOGGER_H
