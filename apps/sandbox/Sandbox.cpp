//  --== Includes ==--
#include <iostream>
#include <Rurouni.hpp>

/*!
 * This is just an application to test recently added functionalities via code usage.
 * It still reflects the actual usage of the engine:
 * There is no main() function needed, as it is part of the engines EntryPoint class.
 * The virtual functions provided by the Game class are to implement the game specific
 * functionalities. All general game functionalities (Loop, Updating, Event handling, Window,
 * Rendering, ...) are handled by the engine.
 */
class Sandbox : public Rurouni::Game
{
public:
    Sandbox()
    {
        std::cout << "Client Game Constructor()" << std::endl;
    }

    virtual ~Sandbox()
    {
        std::cout << "Client Game Destructor()" << std::endl;
    }

    virtual bool ExternalSetup()
    {
        std::cout << "Client Game ExternalSetup()" << std::endl;

        return false;
    }
};

/*!
 * This is the definition of the CreateGame() function stub inside the Game class.
 * It specifies which Game inheriting class should be instantiated inside the EntryPoint.
 *
 * @return Your game.
 */
Rurouni::Game* Rurouni::CreateGame()
{
    return new Sandbox;
}