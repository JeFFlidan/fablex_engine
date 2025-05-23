#pragma once

#include "engine/fwd.h"
#include "core/math.h"

namespace fe::editor
{

class ViewportWindow
{
public:
    void draw() const;
    
    void set_camera(engine::Entity* camera);

private:
    engine::Entity* m_camera = nullptr; // For gizmo
};

}