#pragma once

#include "platform/platform.h"

namespace fe
{

class Timer
{
public:
    static void init();
    static void update();

    static float get_delta_time();

private:
#ifdef WIN32
    inline static LARGE_INTEGER s_ticksPerSecond;
    inline static LARGE_INTEGER s_lastTickCount;
    inline static LARGE_INTEGER s_currentTickCount;

    inline static float s_deltaTime; 
#endif // WIN32
};

}