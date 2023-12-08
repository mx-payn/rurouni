#ifndef RR_LIB_COMMON_TIME_H
#define RR_LIB_COMMON_TIME_H

// std
#include <string>

namespace rr::time {

/** returns current date and time as string.
 *
 *  format is "%FT%H%M%S", e.g "2023-12-08T192100".
 */
std::string get_current_date_and_time();

}  // namespace rr::time

#endif  // !RR_LIB_COMMON_TIME_H
