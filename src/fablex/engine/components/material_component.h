#pragma once

#include "engine/entity/component.h"
#include "asset_manager/fwd.h"

namespace fe::engine
{

class MaterialComponent : public Component
{
    FE_DECLARE_OBJECT(MaterialComponent);
    FE_DECLARE_PROPERTY_REGISTER(MaterialComponent);

public:
    void add_material(asset::Material* material);
    void add_material(UUID materialUUID);

    const std::vector<UUID>& get_material_uuids() const { return m_materialUUIDs; }
    
protected:
    std::vector<UUID> m_materialUUIDs;
};

}