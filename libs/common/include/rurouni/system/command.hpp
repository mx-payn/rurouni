#ifndef RR_LIBS_COMMON_SYSTEM_COMMAND_H
#define RR_LIBS_COMMON_SYSTEM_COMMAND_H

// std
#include <string>
#include "rurouni/system/filesystem.hpp"

namespace rr::system {

/** executes the provided shell command and returns the output */
std::string execute_command(const std::string& command);

}  // namespace rr::system

#endif  // !RR_LIBS_COMMON_SYSTEM_COMMAND_H
