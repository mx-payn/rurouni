// rurouni
#include "rurouni/dev/logger.hpp"
#include "rurouni/runtime/config.h"
#include "rurouni/runtime/logger.hpp"

// std
#include <memory>

int main() {
    rr::dev::LoggerSpecification loggerSpec;
    loggerSpec.Level = rr::dev::LogLevel::Trace;
    loggerSpec.FlushLevel = rr::dev::LogLevel::Trace;
    loggerSpec.LogStdOut = true;
    rr::dev::Logger::init(loggerSpec);

    rr::runtime::LOG = std::make_unique<rr::dev::Logger>("runtime");
    rr::runtime::LOG->info("bin dir ......... : {}", RR_RUNTIME_BIN_DIR);
    rr::runtime::LOG->info("lib dir ......... : {}", RR_RUNTIME_LIB_DIR);
    rr::runtime::LOG->info("archive dir ..... : {}", RR_RUNTIME_ARCHIVE_DIR);
    rr::runtime::LOG->info("shared data dir . : {}",
                           RR_RUNTIME_SHARED_DATA_DIR);
    rr::runtime::LOG->info("shared config dir : {}",
                           RR_RUNTIME_SHARED_CONFIG_DIR);
    rr::runtime::LOG->info("user data dir ... : {}", RR_RUNTIME_USER_DATA_DIR);
    rr::runtime::LOG->info("user config dir . : {}",
                           RR_RUNTIME_USER_CONFIG_DIR);

    return 0;
}
