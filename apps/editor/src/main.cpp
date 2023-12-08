// rurouni
#include "rurouni/dev/logger.hpp"
#include "rurouni/editor/config.h"
#include "rurouni/editor/logger.hpp"
#include "rurouni/math/mat.hpp"
#include "rurouni/math/quaternion.hpp"
#include "rurouni/math/vec.hpp"

// std
#include <memory>

int main() {
    rr::dev::LoggerSpecification loggerSpec;
    loggerSpec.Level = rr::dev::LogLevel::Trace;
    loggerSpec.FlushLevel = rr::dev::LogLevel::Trace;
    loggerSpec.LogStdOut = true;
    rr::dev::Logger::init(loggerSpec);

    rr::runtime::LOG = std::make_unique<rr::dev::Logger>("editor");
    rr::runtime::LOG->info("bin dir ......... : {}", RR_EDITOR_BIN_DIR);
    rr::runtime::LOG->info("lib dir ......... : {}", RR_EDITOR_LIB_DIR);
    rr::runtime::LOG->info("archive dir ..... : {}", RR_EDITOR_ARCHIVE_DIR);
    rr::runtime::LOG->info("shared data dir . : {}", RR_EDITOR_SHARED_DATA_DIR);
    rr::runtime::LOG->info("shared config dir : {}",
                           RR_EDITOR_SHARED_CONFIG_DIR);
    rr::runtime::LOG->info("user data dir ... : {}", RR_EDITOR_USER_DATA_DIR);
    rr::runtime::LOG->info("user config dir . : {}", RR_EDITOR_USER_CONFIG_DIR);

    return 0;
}
