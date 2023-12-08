// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/time.hpp"

namespace rr::time {

std::string get_current_date_and_time() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%FT%H%M%S");
    return ss.str();
}

}  // namespace rr::time
