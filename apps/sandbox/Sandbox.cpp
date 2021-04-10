#include <iostream>
#include <Rurouni.hpp>

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

Rurouni::Game* Rurouni::CreateGame()
{
    return new Sandbox;
}