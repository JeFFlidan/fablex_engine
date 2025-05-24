#pragma once

#include "math.h"
#include "input_enums.h"
#include "platform/platform.h"

namespace fe
{

class Window;

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

struct ViewportState
{
    bool isHovered = false;
    Float2 min = Float2(0, 0);
    Float2 max = Float2(0, 0);
    float width = 0;
    float height = 0;
};

class Input
{
public:
    static void set_mouse_state(MouseState mouseState){ s_mouseState = mouseState; }
    static void set_viewport_state(ViewportState viewportState) { s_viewportState = viewportState; }
    static const ViewportState& get_viewport_state() { return s_viewportState; }
    static void set_cursor_position(float x, float y) { s_mouseState.position = Float2(x, y); }

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

    static Float2 get_position() { return s_mouseState.position; }
    static Float2 get_delta_position() { return s_mouseState.deltaPosition; }

    static bool is_key_pressed(Key key)
    {
#ifdef WIN32
        uint8_t keyCode = UtilsWin32::parse_key(key);
        return UtilsWin32::is_key_down(keyCode) || UtilsWin32::is_key_toggle(keyCode);
#else
        return false;
#endif // WIN32
    }

    static void set_main_window(Window* window) { FE_CHECK(window); s_mainWindow = window; }
    static Window* get_main_window() { return s_mainWindow;}

private:
    inline static MouseState s_mouseState;
    inline static ViewportState s_viewportState;
    inline static Window* s_mainWindow = nullptr;
};

}