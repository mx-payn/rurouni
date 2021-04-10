#ifndef RUROUNI_WIN_ENTRYPOINT_HPP
#define RUROUNI_WIN_ENTRYPOINT_HPP

#include <windows.h>

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
        PSTR lpCmdLine, INT nCmdShow)
{
    std::cout << "Hello from windows_entry!" << std::endl;

    auto game = Rurouni::CreateGame();

    game->InternalSetup();
    game->ExternalSetup();
    game->Run();
    game->Shutdown();

    delete game;

    return 0;
}

#endif //RUROUNI_WIN_ENTRYPOINT_HPP
