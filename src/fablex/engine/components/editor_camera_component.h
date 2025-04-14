#pragma once

#include "camera_component.h"

namespace fe::engine
{

class EditorCameraComponent : public CameraComponent
{
    FE_DECLARE_OBJECT(EditorCameraComponent);

public:
    virtual void update(float deltaTime) override;

private:
    void read_input(float deltaTime);
    void update_matrices();
};

}