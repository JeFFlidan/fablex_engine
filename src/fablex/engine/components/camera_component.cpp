#include "camera_component.h"
#include "core/file_system/archive.h"

namespace fe::engine
{

FE_DEFINE_OBJECT(CameraComponent, Component);

FE_BEGIN_PROPERTY_REGISTER(CameraComponent)
{
    FE_REGISTER_PROPERTY(CameraComponent, zNear, EditAnywhere());
    FE_REGISTER_PROPERTY(CameraComponent, zFar, EditAnywhere());
    FE_REGISTER_PROPERTY(CameraComponent, fov, EditAnywhere());
    FE_REGISTER_PROPERTY(CameraComponent, isActive, EditAnywhere());
    FE_REGISTER_PROPERTY(CameraComponent, movementSpeed, EditAnywhere());
    FE_REGISTER_PROPERTY(CameraComponent, mouseSensitivity, EditAnywhere());
}
FE_END_PROPERTY_REGISTER(CameraComponent)

void CameraComponent::serialize(Archive& archive) const
{
    Component::serialize(archive);

    archive << zNear;
    archive << zFar;
    archive << fov;
    archive << isActive;
    archive << movementSpeed;
    archive << mouseSensitivity;
}

void CameraComponent::deserialize(Archive& archive)
{
    Component::deserialize(archive);

    archive >> zNear;
    archive >> zFar;
    archive >> fov;
    archive >> isActive;
    archive >> movementSpeed;
    archive >> mouseSensitivity;
}

}