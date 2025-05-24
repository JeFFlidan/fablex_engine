#pragma once

#include "engine/entity/component.h"
#include "asset_manager/fwd.h"
#include "core/event.h"

namespace fe::engine
{

class MaterialComponent : public Component
{
    FE_DECLARE_OBJECT(MaterialComponent);
    FE_DECLARE_PROPERTY_REGISTER(MaterialComponent);

public:
    void init(asset::Model* model);

    void add_material(asset::Material* material);
    void add_material(UUID materialUUID);

    bool set_material(asset::Material* material, uint32 index);
    bool set_material(UUID materialUUID, uint32 index);

    const std::vector<UUID>& material_uuids() const { return m_materialUUIDs; }

    virtual void serialize(Archive& archive) const override;
    virtual void deserialize(Archive& archive) override;
    
protected:
    std::vector<UUID> m_materialUUIDs;
};

class MaterialUpdatedEvent : public IEvent
{
public:
    FE_DECLARE_EVENT(MaterialSelectedEvent);

    MaterialUpdatedEvent(MaterialComponent* component) : m_component(component) { }

    MaterialComponent* material_component() const { return m_component; }

private:
    MaterialComponent* m_component;
};

}