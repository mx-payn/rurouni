#ifndef RUROUNI_LINUX_ENTRYPOINT_HPP
#define RUROUNI_LINUX_ENTRYPOINT_HPP

#include <iostream>
#include "rurouni/Game.hpp"

int main()
{
    std::cout << "Hello from linux_entry!" << std::endl;

    auto game = Rurouni::CreateGame();

    game->InternalSetup();
    game->ExternalSetup();
    game->Run();
    game->Shutdown();

    delete game;

    return 0;
}

#endif //RUROUNI_LINUX_ENTRYPOINT_HPP
