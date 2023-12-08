#ifndef RR_LIBS_MATH_H
#define RR_LIBS_MATH_H

#include "rurouni/math/mat.hpp"
#include "rurouni/math/quaternion.hpp"
#include "rurouni/math/vec.hpp"

namespace rr::math {

mat4 ortho(float const& left,
           float const& right,
           float const& bottom,
           float const& top,
           float const& near,
           float const& far);

mat4 rotate(mat4 const& mat, float const& angle, vec3 const& vec);
mat4 scale(mat4 const& mat, vec3 const& vec);
mat4 translate(mat4 const& mat, vec3 const& vec);

// TODO may template this?
float radians(float v);
vec2 radians(const vec2& vec);
vec3 radians(const vec3& vec);
vec4 radians(const vec4& vec);

float degrees(float v);
vec2 degrees(const vec2& vec);
vec3 degrees(const vec3& vec);
vec4 degrees(const vec4& vec);

mat2 inverse(mat2 const& mat);
mat3 inverse(mat3 const& mat);
mat4 inverse(mat4 const& mat);

mat3 toMat3(quat const& x);
mat4 toMat4(quat const& x);

float* value_ptr(vec2& vec);
float const* value_ptr(vec2 const& vec);
float* value_ptr(vec3& vec);
float const* value_ptr(vec3 const& vec);
float* value_ptr(vec4& vec);
float const* value_ptr(vec4 const& vec);

int* value_ptr(ivec2& vec);
int const* value_ptr(ivec2 const& vec);
int* value_ptr(ivec3& vec);
int const* value_ptr(ivec3 const& vec);
int* value_ptr(ivec4& vec);
int const* value_ptr(ivec4 const& vec);

float* value_ptr(mat2& mat);
float const* value_ptr(mat2 const& mat);
float* value_ptr(mat3& mat);
float const* value_ptr(mat3 const& mat);
float* value_ptr(mat4& mat);
float const* value_ptr(mat4 const& mat);

}  // namespace rr::math

#endif  // !RR_LIBS_MATH_H
