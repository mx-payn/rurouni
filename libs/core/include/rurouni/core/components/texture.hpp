#ifndef RR_LIB_CORE_COMPONENTS_TEXTURE_H
#define RR_LIB_CORE_COMPONENTS_TEXTURE_H

// rurouni
#include "rurouni/types/uuid.hpp"

namespace rr::core::components {

struct Texture {
    UUID TextureUUID = UUID();

    Texture(const UUID& textureUUID = UUID()) : TextureUUID(textureUUID) {}
};

}  // namespace rr::core::components

/////////////////////////////////////////////////////////////////
///////////////////  serialization  /////////////////////////////
/////////////////////////////////////////////////////////////////
#include <cereal/cereal.hpp>

namespace rr::core::components {

template <typename Archive>
void serialize(Archive& archive, rr::core::components::Texture& comp) {
    archive(cereal::make_nvp("texture_uuid", comp.TextureUUID));
}

}  // namespace rr::core::components

#endif  // !RR_LIB_CORE_COMPONENTS_TEXTURE_H
