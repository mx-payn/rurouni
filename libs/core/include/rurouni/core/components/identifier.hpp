#ifndef RR_LIBS_CORE_COMPONENTS_IDENTIFIER
#define RR_LIBS_CORE_COMPONENTS_IDENTIFIER

// rurouni
#include "rurouni/types/uuid.hpp"

// std
#include <string>

namespace rr::core::components {

struct Identifier {
    UUID Uuid = UUID();
    std::string Name = std::string();

    Identifier() = default;
    Identifier(const UUID& uuid, const std::string& name)
        : Uuid(uuid), Name(name) {}
};

}  // namespace rr::core::components

/////////////////////////////////////////////////////////////////
///////////////////  serialization  /////////////////////////////
/////////////////////////////////////////////////////////////////
#include "cereal/cereal.hpp"

namespace rr::core::components {

template <class Archive>
void serialize(Archive& archive, rr::core::components::Identifier& comp) {
    archive(cereal::make_nvp("uuid", comp.Uuid),
            cereal::make_nvp("name", comp.Name));
}

}  // namespace rr::core::components

#endif  // !RR_LIBS_CORE_COMPONENTS_IDENTIFIER
