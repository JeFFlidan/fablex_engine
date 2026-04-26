#pragma once

#include "engine/components/fwd.h"
#include "shaders/interops/shader_interop_camera.h"

namespace fe::renderer
{

struct CameraData : ShaderCamera
{
    void fill(engine::CameraComponent* cameraComponent);
};

}