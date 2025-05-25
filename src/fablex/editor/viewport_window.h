#pragma once

#include "common.h"
#include "core/types.h"

namespace fe::editor
{

class ViewportWindow
{
public:
    void draw(const EntitySet& selectedEntities);
    
    void set_camera(engine::Entity* camera);

private:
    engine::Entity* m_camera = nullptr; // For gizmo
    int32 m_gizmoType = -1;

    void draw_gizmo(const EntitySet& selectedEntities);
};

}