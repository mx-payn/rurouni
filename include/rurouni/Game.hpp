#ifndef RUROUNI_GAME_HPP
#define RUROUNI_GAME_HPP

//  --== Includes ==--
#include <iostream>
#include "rr_export.h"

namespace Rurouni {

    /*!
     * The parent class of the client-implemented child class.
     * After instantiation by the EntryPoint, the Game class handles all recurring
     * and non client conserving functionalities.
     * The client is to implement custom functionalities by using the pure virtual functions.
     */
    class RR_EXPORT Game
    {
    public:
//  --== Construction / Destruction ==--
        Game();
        virtual ~Game();

//  --== Runtime ==--
        bool InternalSetup();
        virtual bool ExternalSetup() = 0;
        bool Run();
        bool Shutdown();

//  -- TEMPORARY: Delete when not needed anymore
        static int test_addition(int a, int b)
        {
            return a + b;
        }
    };

    /*!
     * The function stub that gets implemented in the
     * client project.
     *
     * @return Pointer to the Game-deriving class to be executed.
     */
    Game* CreateGame();
}


#endif //RUROUNI_GAME_HPP
