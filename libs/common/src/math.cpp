// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/math.hpp"

// external
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace rr::math {

mat4 ortho(float const& left,
           float const& right,
           float const& bottom,
           float const& top,
           float const& near,
           float const& far) {
    return glm::ortho(left, right, bottom, top, near, far);
}

mat4 rotate(mat4 const& mat, float const& angle, vec3 const& vec) {
    return glm::rotate(mat, angle, vec);
}

mat4 scale(mat4 const& mat, vec3 const& vec) {
    return glm::scale(mat, vec);
}

mat4 translate(mat4 const& mat, vec3 const& vec) {
    return glm::translate(mat, vec);
}

float radians(float v) {
    return glm::radians(v);
}

vec2 radians(const vec2& vec) {
    return glm::radians(vec);
}

vec3 radians(const vec3& vec) {
    return glm::radians(vec);
}

vec4 radians(const vec4& vec) {
    return glm::radians(vec);
}

float degrees(float v) {
    return glm::radians(v);
}

vec2 degrees(const vec2& vec) {
    return glm::radians(vec);
}

vec3 degrees(const vec3& vec) {
    return glm::degrees(vec);
}

vec4 degrees(const vec4& vec) {
    return glm::radians(vec);
}

mat2 inverse(mat2 const& mat) {
    return glm::inverse(mat);
}

mat3 inverse(mat3 const& mat) {
    return glm::inverse(mat);
}

mat4 inverse(mat4 const& mat) {
    return glm::inverse(mat);
}

mat3 toMat3(quat const& x) {
    return glm::toMat3(x);
}

mat4 toMat4(quat const& x) {
    return glm::toMat4(x);
}

float* value_ptr(vec2& vec) {
    return glm::value_ptr(vec);
}
float const* value_ptr(vec2 const& vec) {
    return glm::value_ptr(vec);
}
float* value_ptr(vec3& vec) {
    return glm::value_ptr(vec);
}
float const* value_ptr(vec3 const& vec) {
    return glm::value_ptr(vec);
}
float* value_ptr(vec4& vec) {
    return glm::value_ptr(vec);
}
float const* value_ptr(vec4 const& vec) {
    return glm::value_ptr(vec);
}

int* value_ptr(ivec2& vec) {
    return glm::value_ptr(vec);
}
int const* value_ptr(ivec2 const& vec) {
    return glm::value_ptr(vec);
}
int* value_ptr(ivec3& vec) {
    return glm::value_ptr(vec);
}
int const* value_ptr(ivec3 const& vec) {
    return glm::value_ptr(vec);
}
int* value_ptr(ivec4& vec) {
    return glm::value_ptr(vec);
}
int const* value_ptr(ivec4 const& vec) {
    return glm::value_ptr(vec);
}

float* value_ptr(mat2& mat) {
    return glm::value_ptr(mat);
}
float const* value_ptr(mat2 const& mat) {
    return glm::value_ptr(mat);
}
float* value_ptr(mat3& mat) {
    return glm::value_ptr(mat);
}
float const* value_ptr(mat3 const& mat) {
    return glm::value_ptr(mat);
}
float* value_ptr(mat4& mat) {
    return glm::value_ptr(mat);
}
float const* value_ptr(mat4 const& mat) {
    return glm::value_ptr(mat);
}

}  // namespace rr::math
