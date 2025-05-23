#pragma once

#include "common.h"

namespace fe::editor
{

class OutlinerWindow
{
public:
    void draw(engine::World* world);

    engine::Entity* last_selected_entity() const { return m_lastSelectedEntity; }
    const EntitySet& selected_entities() const { return m_selectedEntities; }

private:
    engine::Entity* m_renamedEntity = nullptr;
    engine::Entity* m_lastSelectedEntity = nullptr;
    engine::Entity* m_selectedByShitfEntity = nullptr;
    bool m_anyItemHovered = false;
    
    EntitySet m_selectedEntities;
    std::vector<engine::Entity*> m_allDrawnEntities;

    void draw_node(engine::Entity* entity);
    void handle_click(engine::Entity* entity);
    void handle_shift_selection();
    void select_children(engine::Entity* entity);
    void deselect_children(engine::Entity* entity);
    void select_entity(engine::Entity* entity);
    void deselect_entity(engine::Entity* entity);
};

}