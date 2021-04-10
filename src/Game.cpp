#include "rurouni/Game.hpp"

namespace Rurouni
{
    Game::Game()
    {
        std::cout << "Engine Game Constructor()" << std::endl;
    }

    Game::~Game()
    {
        std::cout << "Engine Game Destructor()" << std::endl;
    }

    bool Game::InternalSetup()
    {
        std::cout << "Engine Game InternalSetup()" << std::endl;

        return false;
    }

    bool Game::Run()
    {
        std::cout << "Engine Game Run()" << std::endl;

        return false;
    }

    bool Game::Shutdown()
    {
        std::cout << "Engine Game Shutdown()" << std::endl;

        return false;
    }
}