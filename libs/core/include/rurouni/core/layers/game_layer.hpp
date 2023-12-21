#ifndef RR_LIBS_CORE_LAYERS_GAME_LAYER_H
#define RR_LIBS_CORE_LAYERS_GAME_LAYER_H

// rurouni
#include "cereal/cereal.hpp"
#include "rurouni/core/layer.hpp"
#include "rurouni/core/scene_state.hpp"
#include "rurouni/graphics/texture.hpp"
#include "rurouni/math/vec.hpp"

// external
#include <cereal/types/polymorphic.hpp>
#include <entt/entt.hpp>

// std
#include <memory>

namespace rr::core {

class GameLayer : public Layer {
   public:
    GameLayer();
    ~GameLayer();

    virtual void on_render(graphics::BatchRenderer& renderer,
                           AssetManager& assetManager,
                           entt::registry& registry,
                           const SceneState& sceneState) override;

   private:
    friend class cereal::access;
    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::base_class<Layer>(this));
    }
};

}  // namespace rr::core

// Include any archives you plan on using with your type before you register it
// Note that this could be done in any other location so long as it was prior
// to this file being included
// #include <cereal/archives/binary.hpp>
// #include <cereal/archives/xml.hpp>
#include <cereal/archives/json.hpp>

// Register DerivedClassOne
CEREAL_REGISTER_TYPE(rr::core::GameLayer);

#endif  // !RR_LIBS_CORE_LAYERS_GRID_LAYER_H
