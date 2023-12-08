#include "rurouni/dev/logger.hpp"
#include "rurouni/runtime/logger.hpp"
#include <memory>

int main() {
    rr::dev::LoggerSpecification loggerSpec;
    loggerSpec.Level = rr::dev::LogLevel::Trace;
    loggerSpec.FlushLevel = rr::dev::LogLevel::Trace;
    loggerSpec.LogStdOut = true;
    rr::dev::Logger::init(loggerSpec);

    rr::runtime::LOG = std::make_unique<rr::dev::Logger>("runtime");
    rr::runtime::LOG->info("Hello from runtime!");

    return 0;
}
