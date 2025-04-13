#ifdef WIN32

#include "input_reader_win32.h"
#include "core/input_event.h"

namespace fe
{

constexpr uint32 MAX_KEY_COUNT = 256;

InputReaderWin32::InputReaderWin32()
{
    RAWINPUTDEVICE devices[2] = { };

    // Mouse
    devices[0].usUsagePage = 0x01;
    devices[0].usUsage = 0x02;
    devices[0].dwFlags = 0;
    devices[0].hwndTarget = 0;

    // Keyboard
    devices[1].usUsagePage = 0x01;
    devices[1].usUsage = 0x06;
    devices[1].dwFlags = 0;
    devices[1].hwndTarget = 0;

    if (RegisterRawInputDevices(devices, sizeof(devices) / sizeof(devices[0]), sizeof(devices[0])) == FALSE)
        FE_CHECK(0);

    m_allocator.reserve(1024 * 1024, 8);
}

void InputReaderWin32::read_inputs(InputEvent& inputEvent)
{
    MouseState mouseState;

    while (true)
    {
        UINT rawBufferSize = 0;
        UINT result = GetRawInputBuffer(NULL, &rawBufferSize, sizeof(RAWINPUTHEADER));
        assert(result == 0);
        rawBufferSize *= 8;

        if (rawBufferSize == 0)
        {
            inputEvent.set_mouse_state(mouseState);
            set_cursor_position(inputEvent);
            m_allocator.reset();
            return;
        }

        PRAWINPUT rawBuffer = (PRAWINPUT)m_allocator.allocate((size_t)rawBufferSize);
        assert(rawBuffer != nullptr);

        UINT count = GetRawInputBuffer(rawBuffer, &rawBufferSize, sizeof(RAWINPUTHEADER));
        if (count == -1)
            FE_CHECK(0);

        for (UINT currentRaw = 0; currentRaw < count; ++currentRaw)
            parse_raw_input_buffer(rawBuffer[currentRaw], mouseState);
    }
}

void InputReaderWin32::parse_raw_input_buffer(const RAWINPUT& rawInput, MouseState& outMouseState)
{
    if (rawInput.header.dwType == RIM_TYPEKEYBOARD)
    {
        RAWKEYBOARD rawKeyboard = rawInput.data.keyboard;
        if (rawKeyboard.VKey < MAX_KEY_COUNT)
        {
            if (rawKeyboard.Flags == RI_KEY_MAKE)
            {
                // TODO
            }
        }
    }
    else if (rawInput.header.dwType == RIM_TYPEMOUSE)
    {
        RAWMOUSE rawMouse = rawInput.data.mouse;
        if (rawInput.data.mouse.usFlags == MOUSE_MOVE_RELATIVE)
        {
            if (abs(rawMouse.lLastX) < 30000)
            {
                outMouseState.deltaPosition.x += (float)rawMouse.lLastX;
            }
            if (abs(rawMouse.lLastY) < 30000)
            {
                outMouseState.deltaPosition.y += (float)rawMouse.lLastY;
            }
        }

        if (rawMouse.usButtonFlags == RI_MOUSE_LEFT_BUTTON_DOWN)
        {
            outMouseState.pressedButton |= MouseButton::LEFT;
        }
        if (rawMouse.usButtonFlags == RI_MOUSE_RIGHT_BUTTON_DOWN)
        {
            outMouseState.pressedButton |= MouseButton::RIGHT;
        }
        if (rawMouse.usButtonFlags == RI_MOUSE_MIDDLE_BUTTON_DOWN)
        {
            outMouseState.pressedButton |= MouseButton::MIDDLE;
        }
    }
}

void InputReaderWin32::set_cursor_position(InputEvent& outInputEvent) const
{
    POINT point;
    if (!GetCursorPos(&point) || !ScreenToClient((HWND)m_hWnd, &point))
    {
        outInputEvent.set_cursor_position(0.0f, 0.0f);
        return;
    }

    outInputEvent.set_cursor_position(point.x, point.y);
}

}

#endif // WIN32