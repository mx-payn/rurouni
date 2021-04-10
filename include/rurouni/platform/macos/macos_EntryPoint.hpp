#ifndef RUROUNI_MACOS_ENTRYPOINT_HPP
#define RUROUNI_MACOS_ENTRYPOINT_HPP

#include "rr_pch.h"

int main()
{
    std::cout << "Hello from macos_entry!" << std::endl;

    auto game = Rurouni::CreateGame();

    game->InternalSetup();
    game->ExternalSetup();
    game->Run();
    game->Shutdown();

    delete game;

    return 0;
}


#endif //RUROUNI_MACOS_ENTRYPOINT_HPP
