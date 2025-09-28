#pragma once

#include "engine/components/fwd.h"
#include "shaders/shader_interop_renderer.h"

namespace fe::renderer
{

struct CameraData : ShaderCamera
{
    void fill(engine::CameraComponent* cameraComponent);
};

}