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
    void set_material(asset::Material* material);
    void set_material_uuid(UUID materialUUID);

    asset::Material* get_material() const;
    UUID get_material_uuid() const;
    
    bool is_material_loaded() const;

protected:
    UUID m_materialUUID = UUID::INVALID;
};

}