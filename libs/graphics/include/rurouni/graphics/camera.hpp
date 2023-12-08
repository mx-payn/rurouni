#ifndef RR_LIB_GRAPHICS_CAMERA_H
#define RR_LIB_GRAPHICS_CAMERA_H

#include "rurouni/math.hpp"

namespace rr::graphics {

class Camera {
   public:
    Camera();
    Camera(const math::ivec2& viewportSize);
    ~Camera() = default;

    void set_viewport_size(const math::ivec2& size);
    void recalculate_projection();

    float get_orthographic_size() const { return m_OrthographicSize; }
    float get_orthographic_near_clip() const { return m_OrthographicNear; }
    float get_orthographic_far_clip() const { return m_OrthographicFar; }
    float get_orthographic_left() const { return m_OrthographicLeft; }
    float get_orthographic_right() const { return m_OrthographicRight; }
    float get_orthographic_top() const { return m_OrthographicTop; }
    float get_orthographic_bottom() const { return m_OrthographicBottom; }
    float get_orthographic_width() const { return m_OrthographicWidth; }
    float get_orthographic_height() const { return m_OrthographicHeight; }

    void set_orthographic_size(float size) {
        m_OrthographicSize = size;
        recalculate_projection();
    }
    void set_orthographic_near_clip(float near) {
        m_OrthographicNear = near;
        recalculate_projection();
    }
    void set_orthographic_far_clip(float far) {
        m_OrthographicFar = far;
        recalculate_projection();
    }

   public:
    void zoom(float zoom) {
        m_OrthographicSize += zoom;
        recalculate_projection();
    }

    void set_camera_values(float orthoSize, float orthoNear, float orthoFar) {
        m_OrthographicSize = orthoSize;
        m_OrthographicNear = orthoNear;
        m_OrthographicFar = orthoFar;

        recalculate_projection();
    }

    const math::mat4& get_projection() const { return m_Projection; }

   private:
    math::mat4 m_Projection = math::mat4(1.0f);
    float m_AspectRatio = 0.8f;

    float m_OrthographicSize = 1.0f;
    float m_OrthographicNear = -1.0f;
    float m_OrthographicFar = 1.0f;
    float m_OrthographicLeft = -1.0f;
    float m_OrthographicRight = 1.0f;
    float m_OrthographicTop = 1.0f;
    float m_OrthographicBottom = -1.0f;
    float m_OrthographicWidth = 2.0f;
    float m_OrthographicHeight = 2.0f;
};

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_CAMERA_H
