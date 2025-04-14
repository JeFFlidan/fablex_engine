#pragma once

#include "core/window.h"
#include "engine/entity/component.h"

namespace fe::engine
{

class CameraComponent : public Component
{
    FE_DECLARE_OBJECT(CameraComponent);
    FE_DECLARE_PROPERTY_REGISTER(CameraComponent);

public:
    float zNear = 0.1f;
    float zFar = 10000.0f;
    float fov = 50.0f;
    bool isActive = false;
    float movementSpeed = 10.0f;
    float mouseSensitivity = 0.1f;

    Float3 eye = Float3(0.0f, 0.0f, 0.0f);
    Float3 at = Float3(0.0f, 0.0f, 1.0f);
    Float3 up = Float3(0.0f, 1.0f, 0.0f);
    Float4x4 view;
    Float4x4 projection;
    Float4x4 viewProjection;
    Float4x4 inverseView;
    Float4x4 inverseProjection;
    Float4x4 inverseViewProjection;

    // TEMP???
    Window* window = nullptr;

    virtual void serialize(Archive& archive) const override;
    virtual void deserialize(Archive& archive) override;
};

}