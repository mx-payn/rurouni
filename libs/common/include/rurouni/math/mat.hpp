#ifndef RR_LIBS_MATH_MAT_H
#define RR_LIBS_MATH_MAT_H

#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

namespace rr::math {

typedef glm::mat2 mat2;
typedef glm::mat3 mat3;
typedef glm::mat4 mat4;

}  // namespace rr::math

/////////////////////////////////////////////////////////////////
///////////////////  string stream  /////////////////////////////
/////////////////////////////////////////////////////////////////
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

template <typename OStream,
          glm::length_t C,
          glm::length_t R,
          typename T,
          glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix) {
    return os << glm::to_string(matrix);
}

/////////////////////////////////////////////////////////////////
/////////////////////  fmt format  //////////////////////////////
/////////////////////////////////////////////////////////////////
#include <fmt/format.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

template <>
struct fmt::formatter<rr::math::mat2> : fmt::formatter<std::string> {
    auto format(rr::math::mat2 mat, format_context& ctx) const
        -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", glm::to_string(mat));
    }
};
template <>
struct fmt::formatter<rr::math::mat3> : fmt::formatter<std::string> {
    auto format(rr::math::mat3 mat, format_context& ctx) const
        -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", glm::to_string(mat));
    }
};

template <>
struct fmt::formatter<rr::math::mat4> : fmt::formatter<std::string> {
    auto format(rr::math::mat4 mat, format_context& ctx) const
        -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", glm::to_string(mat));
    }
};

/////////////////////////////////////////////////////////////////
///////////////////  serialization  /////////////////////////////
/////////////////////////////////////////////////////////////////
#include <cereal/cereal.hpp>

namespace rr::math {

template <class Archive>
void serialize(Archive& archive, rr::math::mat2& m) {
    archive(m[0], m[1]);
}
template <class Archive>
void serialize(Archive& archive, rr::math::mat3& m) {
    archive(m[0], m[1], m[2]);
}
template <class Archive>
void serialize(Archive& archive, rr::math::mat4& m) {
    archive(m[0], m[1], m[2], m[3]);
}

}  // namespace rr::math

#endif  // !RR_LIBS_MATH_MAT_H
