#pragma once

#include "common.h"

namespace fe::editor
{

class ViewportWindow
{
public:
    void draw(const EntitySet& entitySet) const;
    
    void set_camera(engine::Entity* camera);

private:
    engine::Entity* m_camera = nullptr; // For gizmo
};

}