#pragma once

#include "engine/fwd.h"

namespace fe::editor
{

class PropertiesWindow
{
public:
    void draw(engine::Entity* selectedEntity);

private:
    engine::Entity* m_selectedEntity = nullptr;
};

}