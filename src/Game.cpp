#include "rurouni/Game.hpp"

namespace Rurouni
{
    Game::Game()
    {
        RR_LOG_TRACE("Engine Game Constructor()")
    }

    Game::~Game()
    {
        RR_LOG_TRACE("Engine Game Destructor()")
    }

    bool Game::InternalSetup()
    {
        RR_LOG_SCOPE_FUNCTION(INFO)

        return true;
    }

    bool Game::Run()
    {
        RR_LOG_TRACE("Engine Game Run()")

        return true;
    }

    bool Game::Shutdown()
    {
        RR_LOG_SCOPE_FUNCTION(INFO)

        return false;
    }
}