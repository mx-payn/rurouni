#include <rurouni/tools/Assertion.hpp>
#include <catch2/catch.hpp>

TEST_CASE( "assertion function on different expressions", "[AssertFn]" ) {

    bool exp = Rurouni::Assertion::AssertFn(1 < 2, "test", __LINE__, __FUNCTION__ );
    REQUIRE(exp == true);
    exp = Rurouni::Assertion::AssertFn(1 == 1 , "test", __LINE__, __FUNCTION__ );
    REQUIRE(exp == true);
    exp = Rurouni::Assertion::AssertFn((2 < 1) || (1 < 2), "test", __LINE__, __FUNCTION__ );
    REQUIRE(exp == true);
}
