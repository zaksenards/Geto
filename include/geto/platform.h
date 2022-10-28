#include <cstdlib>
#include <cstdio>

#ifndef GETO_PLATFORM_H
#define GETO_PLATFORM_H

#define MAX_CALLBACKS 3
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

    struct Callbacks
    {
        // Called when resize action performed
        typedef void (*ResizeCallback)(int nx, int ny); 

        // Called after window updates
        typedef void (*UpdateCallback)(float dt);

        // Called when user request for window to close
        typedef void (*CloseCallback)();
    };

    struct GETOAPI platform
    {
        static int init();
        static void stop();
        
        static Window* createWindow(const int w, const int h, const char* title);
        static void destroyWindow(Window* window);
        
        static bool shouldClose(const Window* window);
        static bool addResizeCallback(Window* window, Callbacks::ResizeCallback cbk);

        static bool keyDown(char keyCode, Window* window);
        static void updateEvents(Window* window);
    };
}

#endif//GETO_PLATFORM_H
