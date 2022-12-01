#include <geto/platform.h>
#include <geto/keyboard.h>
#include <winerror.h>
#include <windows.h>
#include <cstdlib>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cmath>
//#include <map>

#define GETO_CLASS_NAME "GETO_WINDOWING_CLASS"
constexpr int GETO_CLOSE_EVENT = WM_USER+1;
constexpr int GETO_KEYBOARD_EVENT = WM_USER+2;
constexpr int GETO_RESIZE_EVENT = WM_USER+3;

const int windowMinimized_width = 160;
const int windowMinimized_height = 28;

LRESULT windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_CLOSE:
            PostMessageA(hWnd, GETO_CLOSE_EVENT, 0, 0);
            break;
        case WM_KEYUP:
        case WM_KEYDOWN:
            {
                PostMessageA(hWnd, GETO_KEYBOARD_EVENT, wParam, lParam);
                break;
            }
        case WM_SIZE:
            PostMessageA(hWnd, GETO_RESIZE_EVENT, wParam, lParam);
            break;
        default:
            return DefWindowProcA(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

namespace geto
{

    struct Window
    {
        int width;
        int height;
        char* title;
        HWND handle;
        HDC msContext;
        bool shouldClose;
        bool isMinimized;
        void* callbacks[MAX_CALLBACKS];
        char keyEvent[MAX_KEYBOARD_KEYS]; 
    };

    struct shared 
    {
        static HINSTANCE hInstance;
        static WNDCLASSEXA wc;
    };

    HINSTANCE shared::hInstance = NULL;
    WNDCLASSEXA shared::wc = {};

    int platform::init()
    {
        //TODO: Check if it's already initialized
        
        HINSTANCE hInstance = GetModuleHandle(NULL);
        WNDCLASSEXA wc = {};

        printf("[*] Trying to load window class...\n");
        if(!GetClassInfoExA(hInstance, GETO_CLASS_NAME, &wc))
        {
            printf("[*] Existent class not found. Creating a new one\n");
            wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
            wc.lpszClassName = GETO_CLASS_NAME;
            wc.cbSize = sizeof(WNDCLASSEX);
            wc.lpfnWndProc = windowProc;
            wc.hInstance = hInstance;
            wc.style = CS_OWNDC;

            if(!RegisterClassExA(&wc))
            {
                DWORD error = GetLastError();

                char msg[80];
                sprintf(msg, "[!] Can't register window class. Error code: %d\n",error);
                fprintf(stderr, msg);
                platform::stop();
                return false;
            }
        }
        shared::hInstance = hInstance;
        shared::wc = wc;
        return true;
    }

    void platform::stop()
    {
        printf("[*] Calling stop\n");
        WNDCLASSEXA wc = {};
        if(GetClassInfoExA(shared::hInstance, GETO_CLASS_NAME, &wc))
        {
            UnregisterClassA(GETO_CLASS_NAME, shared::hInstance);
        }
        if(shared::hInstance)
            FreeLibrary(shared::hInstance);
    }

    Window* platform::createWindow(const int width, const int height, const char* title)
    {
        char msg[80];
        sprintf(msg,"[*] Creating window {w:%d,h:%d,t:%s}\n",width,height,title);
        printf(msg);

        HWND hwnd = CreateWindowExA(
                0,
                GETO_CLASS_NAME,
                title,
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT,
                width, height,
                NULL, NULL,
                shared::hInstance,
                NULL
        );
        if(!hwnd)
        {
            DWORD errorCode = GetLastError();
            sprintf(msg, "[!] Can't create window. error code: %d\n",errorCode);
        }

        //Isaac(TODO): Let the user choose if window should be visible after it's creation
        ShowWindow(hwnd, SW_SHOW);

        Window* window = new Window;
        window->width = width;
        window->handle = hwnd;
        window->height = height;
        window->title = (char*)title;
        window->msContext = GetDC(hwnd);
        window->isMinimized = false;
        window->shouldClose = false;
        memset(window->callbacks, 0, sizeof(window->callbacks));

        return window;
    }

    void platform::destroyWindow(Window* window)
    {
        if(window)
        {
            Callbacks::CloseCallback cbk = (Callbacks::CloseCallback) window->callbacks[Callbacks::WINDOW_CLOSE];
            if(cbk != nullptr)
                cbk(window->title);

            DestroyWindow(window->handle);
            DeleteDC(window->msContext);
        }
    }

    bool platform::shouldClose(const Window* window)
    {
        return window->shouldClose;
    }

    void platform::updateEvents(Window* window)
    {
        MSG msg = {};

        for(int i = 0; i < MAX_KEYBOARD_KEYS;i++)
            window->keyEvent[i] &= ~(1 << 0);

        while(PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
        {
            switch(msg.message)
            {
                case GETO_CLOSE_EVENT:
                    {
                        window->shouldClose = true;
                    }
                    break;
                case GETO_KEYBOARD_EVENT:
                    {
                        char vkCode = (char)msg.wParam;
                        unsigned char isDown =  (msg.lParam >> 31) == 0; 
                        unsigned char wasDown = (msg.lParam >> 30 & (1 << 0)); 
                        unsigned char pressed = (isDown & wasDown); 
                        unsigned char keybit = isDown;
                        //printf("KC: %d | ID: %d | WD: %d\n",vkCode, isDown,wasDown);
                        window->keyEvent[vkCode] = keybit;
                    }
                case GETO_RESIZE_EVENT:
                    {
                        if(msg.wParam == SIZE_MINIMIZED)
                        {
                            window->isMinimized = true;
                            Callbacks::MinimizeCallback cbk = (Callbacks::MinimizeCallback) window->callbacks[Callbacks::WINDOW_MINIMIZE];
                            if(cbk) cbk();
                        }
                        else if(msg.wParam == SIZE_RESTORED && window->isMinimized)
                        {
                            Callbacks::MaximizeCallback cbk = (Callbacks::MaximizeCallback) window->callbacks[Callbacks::WINDOW_MAXIMIZE];
                            window->isMinimized = false;
                            if(cbk) cbk();
                        }
                    }
                    break;
            }

            // Calculates window size
            WINDOWINFO wndInfo = {};
            wndInfo.cbSize = sizeof(WINDOWINFO);
            GetWindowInfo(window->handle, &wndInfo);
            RECT rect = wndInfo.rcWindow;
            int width = (int) abs((rect.left - rect.right));
            int height = (int) abs((rect.top - rect.bottom));

            if(width != window->width || height != window->height)
            {
                Callbacks::ResizeCallback cbk = (Callbacks::ResizeCallback) window->callbacks[Callbacks::WINDOW_RESIZE];
                window->width = width;
                window->height = height;
                if(cbk)
                    cbk(width,height);
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    bool platform::keyDown(char keyCode, Window* window)
    {
        return !window->keyEvent[keyCode] & (1 << 0); 
    }

    bool platform::addCallback(Window* window, void* callback, int type)
    {
        if(!callback)
            return false;
        window->callbacks[type] = callback;
        return true;
    }
}
