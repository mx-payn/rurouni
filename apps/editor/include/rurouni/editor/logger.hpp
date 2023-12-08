#ifndef RR_RUNTIME_LOGGER_H
#define RR_RUNTIME_LOGGER_H

#include "rurouni/dev/logger.hpp"

namespace rr::runtime {

static std::unique_ptr<dev::Logger> LOG = nullptr;

}

#endif  // !RR_RUNTIME_LOGGER_H
