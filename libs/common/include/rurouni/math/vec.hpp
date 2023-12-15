#ifndef RR_LIBS_MATH_VEC_H
#define RR_LIBS_MATH_VEC_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace rr::math {

typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;

typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;

}  // namespace rr::math

/////////////////////////////////////////////////////////////////
///////////////////  string stream  /////////////////////////////
/////////////////////////////////////////////////////////////////
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

template <typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector) {
    return os << glm::to_string(vector);
}

/////////////////////////////////////////////////////////////////
/////////////////////  fmt format  //////////////////////////////
/////////////////////////////////////////////////////////////////
#include <fmt/format.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

template <>
struct fmt::formatter<rr::math::vec2> : fmt::formatter<std::string> {
    auto format(rr::math::vec2 vec, format_context& ctx) const
        -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", glm::to_string(vec));
    }
};

template <>
struct fmt::formatter<rr::math::vec3> : fmt::formatter<std::string> {
    auto format(rr::math::vec3 vec, format_context& ctx) const
        -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", glm::to_string(vec));
    }
};

template <>
struct fmt::formatter<rr::math::vec4> : fmt::formatter<std::string> {
    auto format(rr::math::vec4 vec, format_context& ctx) const
        -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", glm::to_string(vec));
    }
};

/////////////////////////////////////////////////////////////////
///////////////////  serialization  /////////////////////////////
/////////////////////////////////////////////////////////////////
#include <cereal/cereal.hpp>

namespace glm {

template <class Archive>
void serialize(Archive& archive, glm::vec2& v) {
    archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y));
}
template <class Archive>
void serialize(Archive& archive, glm::vec3& v) {
    archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y),
            cereal::make_nvp("z", v.z));
}
template <class Archive>
void serialize(Archive& archive, glm::vec4& v) {
    archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y),
            cereal::make_nvp("z", v.z), cereal::make_nvp("w", v.w));
}
template <class Archive>
void serialize(Archive& archive, glm::ivec2& v) {
    archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y));
}
template <class Archive>
void serialize(Archive& archive, glm::ivec3& v) {
    archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y),
            cereal::make_nvp("z", v.z));
}
template <class Archive>
void serialize(Archive& archive, glm::ivec4& v) {
    archive(cereal::make_nvp("x", v.x), cereal::make_nvp("y", v.y), v.z,
            cereal::make_nvp("z", v.w));
}

}  // namespace glm

#endif  // !RR_LIBS_MATH_VEC_H
