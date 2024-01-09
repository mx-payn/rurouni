#ifndef RR_LIBS_COMMON_ERROR_H
#define RR_LIBS_COMMON_ERROR_H

#include <fmt/core.h>

namespace rr {

struct Error {
    template <class... Args>
    Error(fmt::format_string<Args...> fmt, Args&&... args) {
        Message = fmt::format(fmt, std::forward<Args>(args)...);
    }
    std::string Message = std::string();

    explicit operator bool() const { return Message.empty(); }
};

}  // namespace rr

#endif  // !RR_LIBS_COMMON_ERROR_H
