#pragma once

namespace fe::editor
{

class WindowUI
{
public:
    virtual ~WindowUI() = default;

    virtual bool draw() = 0;
};

}