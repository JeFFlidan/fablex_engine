#pragma once

#include "math.h"
#include "input_enums.h"
#include "platform/platform.h"

namespace fe
{

struct KeyboardState
{
    Key keys = Key::UNKNOWN;
};

struct MouseState
{
    Float2 position{0.0f, 0.0f};
    Float2 deltaPosition{0.0f, 0.0f};
    MouseButton pressedButton{MouseButton::UNKNOWN};
};

class Input
{
public:
    static void set_mouse_state(MouseState mouseState){ m_mouseState = mouseState; }
    static void set_cursor_position(float x, float y) { m_mouseState.position = Float2(x, y); }

    static bool is_mouse_button_pressed(MouseButton mouseButton)
    {
        #ifdef WIN32
        switch (mouseButton)
        {
            case MouseButton::RIGHT:
                return UtilsWin32::is_key_down(VK_RBUTTON);
            case MouseButton::LEFT:
                return UtilsWin32::is_key_down(VK_LBUTTON);
            case MouseButton::MIDDLE:
                return UtilsWin32::is_key_down(VK_MBUTTON);
            default:
                return false;
        }
        #else    
        return false;
        #endif // WIN32
    }

    static Float2 get_position() { return m_mouseState.position; }
    static Float2 get_delta_position() { return m_mouseState.deltaPosition; }

    static bool is_key_pressed(Key key)
    {
#ifdef WIN32
        uint8_t keyCode = UtilsWin32::parse_key(key);
        return UtilsWin32::is_key_down(keyCode) || UtilsWin32::is_key_toggle(keyCode);
#else
        return false;
#endif // WIN32
    }

private:
    inline static MouseState m_mouseState;
};

}