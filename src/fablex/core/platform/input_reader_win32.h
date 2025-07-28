#pragma once

#ifdef WIN32

#include "platform_win32.h"
#include "core/memory/linear_allocator.h"

namespace fe
{

class InputEvent;
struct MouseState;

class InputReaderWin32
{
public:
    InputReaderWin32();

    void set_hwnd(HWND hwnd) { m_hWnd = hwnd; }
    void read_inputs();

private:
    HWND m_hWnd;
    LinearAllocator m_allocator;

    void parse_raw_input_buffer(const RAWINPUT& rawInput, MouseState& outMouseState);
    void set_cursor_position() const;
};

#endif // WIN32

}