#ifndef RR_LIB_CORE_COMPONENTS_ORTHOGRAPHIC_PROJECTION_H
#define RR_LIB_CORE_COMPONENTS_ORTHOGRAPHIC_PROJECTION_H

// rurouni
#include "rurouni/math.hpp"

// external
#include <cereal/access.hpp>
#include <cereal/cereal.hpp>

namespace rr::core::components {

struct OrthographicProjection {
   private:
    float m_Size = 1.0f;
    float m_Near = -1.0f;
    float m_Far = 1.0f;

    math::mat4 m_Projection = math::mat4(1.0f);
    bool m_ProjectionDirty = true;
    float m_AspectRatio = 0.8f;

    void recalculate_projection() {
        float left = -m_Size * m_AspectRatio * 0.5f;
        float right = m_Size * m_AspectRatio * 0.5f;
        float bottom = -m_Size * 0.5f;
        float top = m_Size * 0.5f;
        float width = right - left;
        float height = top - bottom;

        m_Projection = math::ortho(left, right, bottom, top, m_Near, m_Far);
        m_ProjectionDirty = false;
    }

   public:
    float get_size() const { return m_Size; }
    float get_near_clip() const { return m_Near; }
    float get_far_clip() const { return m_Far; }
    const math::mat4& get_projection() {
        if (m_ProjectionDirty)
            recalculate_projection();

        return m_Projection;
    }

    void set_aspect_ratio(float aspectRatio) {
        m_AspectRatio = aspectRatio;
        m_ProjectionDirty = true;
    }
    void set_aspect_ratio(const math::ivec2& viewportSize) {
        m_AspectRatio = (float)viewportSize.x / (float)viewportSize.y;
        m_ProjectionDirty = true;
    }
    void set_size(float size) {
        m_Size = size;
        m_ProjectionDirty = true;
    }
    void set_near_clip(float near) {
        m_Near = near;
        m_ProjectionDirty = true;
    }
    void set_far_clip(float far) {
        m_Far = far;
        m_ProjectionDirty = true;
    }

    OrthographicProjection() = default;
    OrthographicProjection(const math::ivec2& viewportSize,
                           float size,
                           float near,
                           float far)
        : m_Size(size), m_Near(near), m_Far(far) {
        m_AspectRatio = (float)viewportSize.x / (float)viewportSize.y;
        recalculate_projection();
    }
    OrthographicProjection(float aspectRatio, float size, float near, float far)
        : m_AspectRatio(aspectRatio), m_Size(size), m_Near(near), m_Far(far) {
        recalculate_projection();
    }

   private:
    friend class cereal::access;
    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("size", m_Size),
                cereal::make_nvp("near", m_Near),
                cereal::make_nvp("far", m_Far));
        m_ProjectionDirty = true;
    }
};

}  // namespace rr::core::components

#endif  // !RR_LIB_CORE_COMPONENTS_ORTHOGRAPHIC_PROJECTION_H
