#include "timer.h"

namespace fe
{

void Timer::init()
{
#ifdef WIN32
    QueryPerformanceFrequency(&s_ticksPerSecond);
    QueryPerformanceCounter(&s_lastTickCount);
#endif // WIN32
}

void Timer::update()
{
	QueryPerformanceCounter(&s_currentTickCount);
	uint64_t elapsedTicks = s_currentTickCount.QuadPart - s_lastTickCount.QuadPart;
	uint64_t elapsedTicksMicroseconds = elapsedTicks * 1000000 / s_ticksPerSecond.QuadPart;
	s_deltaTime = (float)elapsedTicksMicroseconds / 1000000.0f;
	s_lastTickCount = s_currentTickCount;
}

float Timer::get_delta_time()
{
    return s_deltaTime;
}

}