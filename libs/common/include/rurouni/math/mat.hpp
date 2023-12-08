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
///////////////////  serialization  /////////////////////////////
/////////////////////////////////////////////////////////////////
#include <cereal/cereal.hpp>

namespace glm {

// glm matrices serialization
template <class Archive>
void serialize(Archive& archive, glm::mat2& m) {
    archive(m[0], m[1]);
}
template <class Archive>
void serialize(Archive& archive, glm::mat3& m) {
    archive(m[0], m[1], m[2]);
}
template <class Archive>
void serialize(Archive& archive, glm::mat4& m) {
    archive(m[0], m[1], m[2], m[3]);
}

}  // namespace glm

#endif  // !RR_LIBS_MATH_MAT_H
