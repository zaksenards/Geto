#include <geto/platform.h>
#include <geto/keyboard.h>
#include <cstdlib>
#include <cstdio>

using namespace geto;

int main(int argc, char* argv[])
{
    if(!platform::init())
        return 0;

    Window* window = platform::createWindow(800,600,"Geto Window");

    while(!platform::shouldClose(window))
        platform::updateEvents(window);

    platform::destroyWindow(window);
    platform::stop();
    return 0;
}
