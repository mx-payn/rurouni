#ifndef RR_LIBS_CORE_SCENE_STATE_H
#define RR_LIBS_CORE_SCENE_STATE_H

#include "rurouni/math/vec.hpp"

#include <entt/entt.hpp>

namespace rr::core {

struct SceneState {
    math::ivec2 ViewportSize_px;

    // cameras
    entt::entity LayerCameraId = entt::null;
    entt::entity OverlayCameraId = entt::null;

    // grid
    math::ivec2 CellCount;
    math::ivec2 CellSize_px;
};

}  // namespace rr::core

/////////////////////////////////////////////////////////////////
///////////////////  serialization  /////////////////////////////
/////////////////////////////////////////////////////////////////
#include "cereal/cereal.hpp"

namespace rr::core {

template <class Archive>
void serialize(Archive& archive, rr::core::SceneState& state) {
    archive(cereal::make_nvp("layer_camera_entt_id", state.LayerCameraId),
            cereal::make_nvp("overlay_camera_entt_id", state.OverlayCameraId),
            cereal::make_nvp("cell_count_y", state.CellCount.y));
}

}  // namespace rr::core

namespace entt {

template <class Archive>
uint32_t save_minimal(Archive const&, entt::entity const& entity) {
    return static_cast<uint32_t>(entity);
}

template <class Archive>
void load_minimal(Archive const&, entt::entity& entity, uint32_t const& value) {
    entity = static_cast<entt::entity>(value);
}

}  // namespace entt

#endif  // !RR_LIBS_CORE_SCENE_STATE_H
