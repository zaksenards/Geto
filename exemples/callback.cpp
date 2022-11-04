#include <geto/platform.h>
#include <geto/keyboard.h>
#include <cstdlib>
#include <cstdio>

using namespace geto;

void onResize(int x, int y);
void onClose(const char* title);

int main(int argc, char* argv[])
{
    if(!platform::init())
        return 0;

    Window* window = platform::createWindow(800,600,"Geto Window");

    platform::addCallback(window, onResize, Callbacks::WINDOW_RESIZE);
    platform::addCallback(window, onClose, Callbacks::WINDOW_CLOSE);

    while(!platform::shouldClose(window) & !platform::keyDown(VK_ESCAPE, window))
        platform::updateEvents(window);

    platform::destroyWindow(window);
    platform::stop();
    return 0;
}

void onResize(int x, int y)
{
    printf("\n[*] New window size is %d x %d\n",x,y);
}

void onClose(const char* title)
{
    printf("Window window %s was closed\n",title);
}
