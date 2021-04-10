#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <rurouni/Game.hpp>

TEST_CASE( "Quick check", "[test_addition]" ) {
    int a = 3;
    int b = 2;
    int c = Rurouni::Game::test_addition(a, b);

    REQUIRE( c == 5 );
    //REQUIRE( moment == Approx(4.666666) );
}
