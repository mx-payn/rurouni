#ifndef RR_LIBS_CORE_COMPONENTS_TRANSFORM_H
#define RR_LIBS_CORE_COMPONENTS_TRANSFORM_H

// rurouni
#include "rurouni/math.hpp"

// external
#include <cereal/access.hpp>
#include <cereal/cereal.hpp>

namespace rr::core::components {

struct Transform {
   private:
    math::vec3 m_Translation = math::vec3(0.0f);
    math::vec3 m_Scale = math::vec3(1.0f);
    math::vec3 m_Rotation = math::vec3(0.0f);

    math::mat4 m_Transform = math::mat4(1.0f);
    bool m_TransformDirty = true;

   public:
    Transform() = default;
    Transform(const math::vec3& translation) : m_Translation(translation) {}
    Transform(const math::vec3& translation,
              const math::vec3& scale,
              const math::vec3& rotation)
        : m_Translation(translation), m_Scale(scale), m_Rotation(rotation) {}

    const math::vec3& get_translation() const { return m_Translation; }
    const math::vec3& get_scale() const { return m_Scale; }
    const math::vec3& get_rotation() const { return m_Rotation; }
    const math::mat4& get_transform() {
        if (m_TransformDirty)
            calculate_transform();

        return m_Transform;
    }

    void set_translation(const math::vec3& translation) {
        m_Translation = translation;
        m_TransformDirty = true;
    }
    void set_scale(const math::vec3& scale) {
        m_Scale = scale;
        m_TransformDirty = true;
    }
    void set_rotation(const math::vec3& rotation) {
        m_Rotation = rotation;
        m_TransformDirty = true;
    }

    void calculate_transform() {
        math::mat4 _rotation = math::toMat4(math::quat(m_Rotation));

        m_Transform = math::translate(math::mat4(1.0f), m_Translation) *
                      _rotation * math::scale(math::mat4(1.0f), m_Scale);
        m_TransformDirty = false;
    }

    /////////////////////////////////////////////////////////////////
    ///////////////////  serialization  /////////////////////////////
    /////////////////////////////////////////////////////////////////
   private:
    friend class cereal::access;
    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("translation", m_Translation),
                cereal::make_nvp("scale", m_Scale),
                cereal::make_nvp("rotation", m_Rotation));
        m_TransformDirty = true;
    }
};

}  // namespace rr::core::components

#endif  // !RR_LIBS_CORE_COMPONENTS_TRANFORM_H
