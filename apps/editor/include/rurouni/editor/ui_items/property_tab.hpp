#ifndef RR_EDITOR_UI_ITEMS_PROPERTY_TAB_H
#define RR_EDITOR_UI_ITEMS_PROPERTY_TAB_H

#include "rurouni/core/asset_manager.hpp"
#include "rurouni/core/components/identifier.hpp"
#include "rurouni/types/uuid.hpp"

#include <entt/entt.hpp>

namespace rr::editor::ui {

class PropertyTab {
   public:
    PropertyTab(const std::string& name) : m_Name(name) {}

    virtual void draw(entt::registry& registry,
                      core::AssetManager& assetManager) = 0;
    virtual const UUID& get_id() const = 0;

    const std::string& get_name() { return m_Name; }

   private:
    std::string m_Name;
};

/*
class EntityProperty : public PropertyTab {
   public:
    EntityProperty(entt::registry& registry,
                   entt::entity entity,
                   const std::string& name = "entity")
        : PropertyTab(name), m_Entity(entity) {
        auto& identification =
registry.get<core::components::Identifier>(m_Entity); m_Id =
identification.Uuid;
    }

    virtual void draw(entt::registry& registry,
                      core::AssetManager& assetManager) override;
    virtual const UUID& get_id() const override;

   private:
    void draw_component_add_popup(entt::registry& registry,
                                  entt::entity entityId);

   private:
    entt::entity m_Entity;
    UUID m_Id;
};
*/

class TextureProperty : public PropertyTab {
   public:
    TextureProperty(const UUID& id, const std::string& name = "texture")
        : PropertyTab(name), m_Id(id) {}

    virtual void draw(entt::registry& registry,
                      core::AssetManager& assetManager) override;
    virtual const UUID& get_id() const override;

   private:
    UUID m_Id;
};

class SpriteProperty : public PropertyTab {
   public:
    SpriteProperty(const UUID& spriteId,
                   const UUID& textureId,
                   const std::string& name = "sprite")
        : PropertyTab(name), m_SpriteId(spriteId), m_TextureId(textureId) {}

    virtual void draw(entt::registry& registry,
                      core::AssetManager& assetManager) override;
    virtual const UUID& get_id() const override;

   private:
    UUID m_SpriteId;
    UUID m_TextureId;
};

}  // namespace rr::editor::ui

#endif  // !RR_EDITOR_UI_ITEMS_PROPERTY_TAB_H
