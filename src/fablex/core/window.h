#pragma once

#include "math.h"
#include "platform/platform.h"
#include <string>

namespace fe
{

struct WindowCreateInfo
{
    std::string windowTitle{"Default"};
    uint32 width{1280};
    uint32 height{720};
    bool isResizable{false};
};

struct WindowInfo
{
    struct
    {
        void* hWnd{nullptr};
    } win32Window;

    // TODO: Add window handles for other OS

    uint32 minWidth{700};
    uint32 minHeight{400};
    uint32 maxWidth{3840};
    uint32 maxHeight{2160};

    uint32 width{0};
    uint32 height{0};
};

class Window
{
public:
    void init(const WindowCreateInfo& createInfo);
    bool process_message();
    void close();

    bool is_running() const { return m_isRunning; }

    const WindowInfo& get_info() const { return m_windowInfo; }
    void set_width(uint32 width);
    void set_height(uint32 height);

private:
#ifdef WIN32
    struct
    {
        HINSTANCE hInstance;
        RECT windowRect;
        RECT clientRect;
        const char* className;
    } m_win32WindowData;

    InputReaderWin32 m_inputReaderWin32;
#endif // WIN32

    WindowInfo m_windowInfo;
    bool m_isRunning{false};

    Float2 get_cursor_relative_to_window() const;
};

}
