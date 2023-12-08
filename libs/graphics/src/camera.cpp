// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/graphics/camera.hpp"
#include "rurouni/math.hpp"

namespace rr::graphics {

Camera::Camera() {
    recalculate_projection();
}

Camera::Camera(const math::ivec2& viewportSize) {
    m_AspectRatio = (float)viewportSize.x / (float)viewportSize.y;

    recalculate_projection();
}

void Camera::set_viewport_size(const math::ivec2& size) {
    m_AspectRatio = (float)size.x / (float)size.y;

    recalculate_projection();
}

void Camera::recalculate_projection() {
    m_OrthographicLeft = -m_OrthographicSize * m_AspectRatio * 0.5f;
    m_OrthographicRight = m_OrthographicSize * m_AspectRatio * 0.5f;
    m_OrthographicBottom = -m_OrthographicSize * 0.5f;
    m_OrthographicTop = m_OrthographicSize * 0.5f;
    m_OrthographicWidth = m_OrthographicRight - m_OrthographicLeft;
    m_OrthographicHeight = m_OrthographicTop - m_OrthographicBottom;

    m_Projection = math::ortho(m_OrthographicLeft, m_OrthographicRight,
                               m_OrthographicBottom, m_OrthographicTop,
                               m_OrthographicNear, m_OrthographicFar);
}

}  // namespace rr::graphics
