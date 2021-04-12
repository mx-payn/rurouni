#ifndef RUROUNI_ENTRYPOINT_HPP
#define RUROUNI_ENTRYPOINT_HPP

// We actually don't need to #import Game.hpp here, as
// EntryPoint is always listed BELOW Game.hpp inside the
// API header file Rurouni.hpp and EntryPoint is header
// only, so there is also no need for a forward-declaration
// as it will not be compiled into an .obj file.
// Pretty neat.
#include "rr_pch.h"

int main(int argc, char** argv)
{
    RR_LOG_INIT(argc, argv, "everything.log")
    RR_LOG_SCOPE_FUNCTION(2)

    auto game = Rurouni::CreateGame();

    // --==> Game Runtime Cycle
    game->InternalSetup();
    game->ExternalSetup();
    game->Run();
    game->Shutdown();
    // <==--

    delete game;

    RR_LOG_DEBUG("all went well! shutting down gracefully...")
    return 0;
}

#endif //RUROUNI_ENTRYPOINT_HPP
