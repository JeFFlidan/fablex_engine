#include "light_components.h"
#include "engine/entity/entity.h"
#include "core/file_system/archive.h"

namespace fe::engine
{

FE_DEFINE_OBJECT(LightComponent, ShaderEntityComponent);
FE_BEGIN_PROPERTY_REGISTER(LightComponent)
{
    FE_REGISTER_PROPERTY(LightComponent, color, EditAnywhere(), Color());
    FE_REGISTER_PROPERTY(LightComponent, intensity, EditAnywhere(), ClampMin(0.0f), ClampMax(1000.0f));
}
FE_END_PROPERTY_REGISTER(LightComponent)

FE_DEFINE_OBJECT(DirectionalLightComponent, LightComponent);
FE_BEGIN_PROPERTY_REGISTER(DirectionalLightComponent)
{

}
FE_END_PROPERTY_REGISTER(DirectionalLightComponent)

void LightComponent::fill_shader_data(ShaderEntity& outShaderEntity) const
{
    outShaderEntity.init();

    outShaderEntity.set_color(Float4(color * intensity));
    outShaderEntity.position = m_entity->get_position();
}

void LightComponent::serialize(Archive& archive) const
{
    ShaderEntityComponent::serialize(archive);

    archive << color;
    archive << intensity;
}

void LightComponent::deserialize(Archive& archive)
{
    ShaderEntityComponent::deserialize(archive);

    archive >> color;
    archive >> intensity;
}

void DirectionalLightComponent::fill_shader_data(ShaderEntity& outShaderEntity) const
{
    LightComponent::fill_shader_data(outShaderEntity);

    Vector baseVec = Vector3::create(0, -1, 0);
    Float3 direction = Vector3::normalize(Vector3::transform_normal(baseVec, m_entity->get_world_transform()));

    outShaderEntity.set_type(SHADER_ENTITY_TYPE_DIRECTIONAL_LIGHT);
    outShaderEntity.set_direction(direction);
}

}