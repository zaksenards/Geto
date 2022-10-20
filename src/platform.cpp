#include <geto/platform.h>
#include <geto/keyboard.h>
#include <winerror.h>
#include <windows.h>
#include <cstdlib>
#include <cstdio>

#ifdef DEBUG_BUILD
#define printerr(msg) fprintf(stderr, msg)
#define printlog(msg) fprintf(stdout, msg)
#endif

#define GETO_CLASS_NAME "GETO_WINDOWING_CLASS"
constexpr int GETO_CLOSE_EVENT = WM_USER+1;
constexpr int GETO_KEYBOARD_EVENT = WM_USER+2;

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
        char* tilte;
        HWND handle;
        HDC msContext;
        bool shouldClose;
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
        HINSTANCE hInstance = GetModuleHandle(NULL);
        WNDCLASSEXA wc = {};

        printlog("[*] Trying to load window class...\n");
        if(!GetClassInfoExA(hInstance, GETO_CLASS_NAME, &wc))
        {
            printlog("[*] [!] existent class not found. Creating a new one\n");
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
                printerr(msg);
                return false;
            }
        }
        shared::hInstance = hInstance;
        shared::wc = wc;
        return true;
    }

    void platform::stop()
    {

    }

    Window* platform::createWindow(const int width, const int height, const char* title)
    {
        char msg[80];
        sprintf(msg,"[*] Creating window {w:%d,h:%d,t:%s}",width,height,title);
        printlog(msg);

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
            sprintf(msg, "Can't create window. error code: %d\n",errorCode);
            printerr(msg);
        }
        ShowWindow(hwnd, SW_SHOW);

        Window* window = new Window;
        window->width = width;
        window->handle = hwnd;
        window->height = height;
        window->tilte = (char*)title;
        window->msContext = GetDC(hwnd);
        window->shouldClose = false;
        return window;
    }

    void platform::destroyWindow(Window* window)
    {
        if(window)
        {
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
                    break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    bool platform::keyDown(char keyCode, Window* window)
    {
        return !window->keyEvent[keyCode] & (1 << 0); 
    }
}
