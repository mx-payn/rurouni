#ifndef RR_LIBS_MATH_QUAT_H
#define RR_LIBS_MATH_QUAT_H

#include <glm/gtc/quaternion.hpp>

namespace rr::math {

typedef glm::tquat<float> quat;

}

/////////////////////////////////////////////////////////////////
///////////////////  string stream  /////////////////////////////
/////////////////////////////////////////////////////////////////
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

template <typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion) {
    return os << glm::to_string(quaternion);
}

/////////////////////////////////////////////////////////////////
/////////////////////  fmt format  //////////////////////////////
/////////////////////////////////////////////////////////////////
#include <fmt/format.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

template <>
struct fmt::formatter<rr::math::quat> : fmt::formatter<std::string> {
    auto format(rr::math::quat quat, format_context& ctx) const
        -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", glm::to_string(quat));
    }
};

/////////////////////////////////////////////////////////////////
///////////////////  serialization  /////////////////////////////
/////////////////////////////////////////////////////////////////
#include <cereal/cereal.hpp>

namespace rr::math {

template <class Archive>
void serialize(Archive& archive, rr::math::quat& q) {
    archive(cereal::make_nvp("x", q.x), cereal::make_nvp("y", q.y),
            cereal::make_nvp("z", q.z), cereal::make_nvp("w", q.w));
}

}  // namespace rr::math

#endif  // !RR_LIBS_MATH_QUAT_H
