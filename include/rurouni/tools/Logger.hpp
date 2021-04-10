#ifndef RUROUNI_LOGGER_HPP
#define RUROUNI_LOGGER_HPP

#include "rr_export.h"
#include "loguru.hpp"

namespace Rurouni {

    /*!
     *  A static logging class wrapping macros from a logging library.
     *  Current library      : loguru
     *          documentation: https://emilk.github.io/loguru/index.html#logging/verbositylevels
     *  Loggers for the Engine and Client are present and can be used
     *  by the defined macros outside the namespace. Macros will expand
     *  to nothing on 'Release' builds.
     */
    class RR_EXPORT Logger
    {
    public:
        /**
         *  sets the logging pattern, initializes the loggers and their
         *  logging levels. Also sets a file to print to, if wished.
         */
        static void Init(int argc = 0, char** argv = {}, const std::string& outPath = "")
        {
            // set the default logging verbosity
            // can be overwritten by user via -v [level] flag
            loguru::g_stderr_verbosity = 9;
            loguru::init(argc, argv);
            if (!outPath.empty())
                loguru::add_file(outPath.c_str(), loguru::Append, loguru::Verbosity_MAX);
        }

    private:
        Logger() = default;
    };

}

//  --== Macros ==--
#ifdef RR_ENABLE_LOG
#   define RR_LOG_INIT(...) ::Rurouni::Logger::Init(__VA_ARGS__);

#   define RR_LOG_ERROR(...) LOG_F(ERROR, __VA_ARGS__);
#   define RR_LOG_WARN(...)  LOG_F(WARNING, __VA_ARGS__);
#   define RR_LOG_INFO(...)  LOG_F(INFO, __VA_ARGS__);
#   define RR_LOG_DEBUG(...) LOG_F(1, __VA_ARGS__);
#   define RR_LOG_TRACE(...) LOG_F(2, __VA_ARGS__);

#   define RR_LOG_SCOPE_F(...) LOG_SCOPE_F(__VA_ARGS__);
#   define RR_LOG_SCOPE_FUNCTION(v) LOG_SCOPE_FUNCTION(v);
#else
#   define RR_LOG_INIT(...)

#   define RR_LOG_ERROR(...)
#   define RR_LOG_WARN(...)
#   define RR_LOG_INFO(...)
#   define RR_LOG_DEBUG(...)
#   define RR_LOG_TRACE(...)

#   define RR_LOG_SCOPE_F(v)
#   define RR_LOG_SCOPE_FUNCTION(v)
#endif

#endif //RUROUNI_LOGGER_HPP
