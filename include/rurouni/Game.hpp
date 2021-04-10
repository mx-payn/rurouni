#ifndef RUROUNI_GAME_HPP
#define RUROUNI_GAME_HPP

#include <iostream>

namespace Rurouni {

    class Game
    {
    public:
        Game();
        virtual ~Game();

        bool InternalSetup();
        virtual bool ExternalSetup() = 0;
        bool Run();
        bool Shutdown();

        static int test_addition(int a, int b)
        {
            return a + b;
        }

    };

    Game* CreateGame();
}


#endif //RUROUNI_GAME_HPP
