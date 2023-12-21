#ifndef RR_LIBS_CORE_LAYER_H
#define RR_LIBS_CORE_LAYER_H

// rurouni
#include "cereal/cereal.hpp"
#include "entt/entity/fwd.hpp"
#include "rurouni/core/asset_manager.hpp"
#include "rurouni/core/scene_state.hpp"
#include "rurouni/graphics/batch_renderer.hpp"

// external
#include <entt/entt.hpp>

namespace rr::core {

class Layer {
   public:
    Layer(const std::string& name) : m_Name(name) {}
    virtual ~Layer() {}

    virtual void on_attach() {}
    virtual void on_detach() {}

    virtual void on_render(graphics::BatchRenderer& renderer,
                           AssetManager& assetManager,
                           entt::registry& registry,
                           const SceneState& sceneState) = 0;

    const std::string& get_name() const { return m_Name; }

   private:
    std::string m_Name;

   private:
    friend class cereal::access;
    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("name", m_Name));
    }
};

}  // namespace rr::core

#endif  // !RR_LIBS_CORE_LAYER_H
