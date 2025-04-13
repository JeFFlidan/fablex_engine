#pragma once

#include "math.h"
#include "event.h"
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

class InputEvent : public IEvent
{
public:
    FE_DECLARE_EVENT(InputEvent);

    void set_mouse_state(MouseState mouseState){ m_mouseState = mouseState; }
    void set_cursor_position(float x, float y) { m_mouseState.position = Float2(x, y); }

    bool is_mouse_button_pressed(MouseButton mouseButton) const
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

    Float2 get_position() const { return m_mouseState.position; }
    Float2 get_delta_position() const { return m_mouseState.deltaPosition; }

    bool is_key_pressed(Key key) const
    {
#ifdef WIN32
        uint8_t keyCode = UtilsWin32::parse_key(key);
        return UtilsWin32::is_key_down(keyCode) || UtilsWin32::is_key_toggle(keyCode);
#else
        return false;
#endif // WIN32
    }

private:
    MouseState m_mouseState;
};

}