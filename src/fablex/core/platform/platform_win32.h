#pragma once

#ifdef WIN32

#ifndef NOMINMAX
    #define NOMINMAX
#endif // NOMINMAX

#ifndef NOGDI
    #define NOGDI
#endif // NOGDI

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#include <windows.h>

#endif // WIN32