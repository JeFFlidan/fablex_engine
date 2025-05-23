#pragma once

#include "engine/fwd.h"
#include <unordered_set>

namespace fe::editor
{

class OutlinerWindow
{
public:
    void draw(engine::World* world);

private:
    engine::Entity* m_renamedEntity = nullptr;
    engine::Entity* m_lastSelectedEntity = nullptr;
    engine::Entity* m_selectedByShitfEntity = nullptr;
    bool m_anyItemHovered = false;
    
    std::unordered_set<engine::Entity*> m_selectedEntities;
    std::vector<engine::Entity*> m_allDrawnEntities;

    void draw_node(engine::Entity* entity);
    void handle_click(engine::Entity* entity);
    void handle_shift_selection();
    void select_children(engine::Entity* entity);
    void deselect_children(engine::Entity* entity);
};

}