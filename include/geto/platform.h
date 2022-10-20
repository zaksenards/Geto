#include <cstdlib>
#include <cstdio>

#ifndef GETO_PLATFORM_H
#define GETO_PLATFORM_H

#define PLATFORM_WINDOWS
#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    #ifdef PLATFORM_IMPLEMENTATION
        #define GETOAPI __declspec(dllexport)
    #else
        #define GETOAPI __declspec(dllimport)
    #endif
#endif

namespace geto
{
    struct GETOAPI Window;

    struct context
    {
        bool  oglContext;
        float oglVersion;
    };

    struct GETOAPI platform
    {
        static int init();
        static void stop();
        
        static Window* createWindow(const int w, const int h, const char* title);
        static void destroyWindow(Window* window);
        
        static bool shouldClose(const Window* window);

        static bool keyDown(char keyCode, Window* window);
        static void updateEvents(Window* window);

    };
}

#endif//GETO_PLATFORM_H
