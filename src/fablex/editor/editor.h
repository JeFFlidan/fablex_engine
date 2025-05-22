#pragma once

#include "core/window.h"

namespace fe::editor
{

class Editor
{
public:
    Editor(Window* window);
    ~Editor();

    void draw();

private:
    Window* m_window = nullptr;
};

}