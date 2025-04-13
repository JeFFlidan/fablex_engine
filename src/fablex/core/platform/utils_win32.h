#pragma once

#ifdef WIN32

#include "core/types.h"
#include "platform_win32.h"
#include "core/input_enums.h"

namespace fe
{

class UtilsWin32
{
public:
    static Key parse_key(WPARAM wParam);
    static uint8 parse_key(Key key);
    static bool is_key_down(uint8 keyCode);
    static bool is_key_toggle(uint8 keyCode);
};

}

#endif // WIN32