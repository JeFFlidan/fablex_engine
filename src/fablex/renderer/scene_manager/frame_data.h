#pragma once

#include "shaders/interops/shader_interop_frame.h"

namespace fe::renderer
{

class SceneManager;

struct FrameData : FrameUB
{
    void fill(const SceneManager* sceneManager);
};

}