#include <engine/window.h>

int main(int argc, char** argv)
{
    auto window = SDLarria::GameWindow::GameWindow();

    window.Update();
    return 0;
}