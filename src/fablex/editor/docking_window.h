#pragma once

namespace fe::editor
{

class DockingWindow
{
public:
    DockingWindow();
    ~DockingWindow() = default;

    void draw() const;
};

}