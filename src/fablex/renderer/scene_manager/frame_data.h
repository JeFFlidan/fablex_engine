#pragma once

#include "shaders/shader_interop_renderer.h"

namespace fe::renderer
{

class SceneManager;

struct FrameData : FrameUB
{
    void fill(const SceneManager* sceneManager);
};

}