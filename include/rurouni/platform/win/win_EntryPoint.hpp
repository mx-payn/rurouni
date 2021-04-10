#ifndef RUROUNI_WIN_ENTRYPOINT_HPP
#define RUROUNI_WIN_ENTRYPOINT_HPP

#include "rr_pch.h"
#include <windows.h>

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
        PSTR lpCmdLine, INT nCmdShow)
{
    RR_LOG_INIT(argc, argv)
    RR_LOG_SCOPE_FUNCTION(2)
    RR_LOG_DEBUG("you are entering through the WINDOWS entrypoint")

    auto game = Rurouni::CreateGame();

    game->InternalSetup();
    game->ExternalSetup();
    game->Run();
    game->Shutdown();

    delete game;

    RR_LOG_DEBUG("all went well! shutting down gracefully")
    return 0;
}

#endif //RUROUNI_WIN_ENTRYPOINT_HPP
