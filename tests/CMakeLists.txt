# Testing library
FetchContent_Declare(
  catch
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.9.1)
FetchContent_MakeAvailable(catch)
# Adds Catch2::Catch2

# Tests need to be added as executables first
add_executable(rurouni_test
        test_game.cpp
        test_assertion.cpp
        test_pool_allocator.cpp
        )

# I'm using C++17 in the test
target_compile_features(rurouni_test PRIVATE cxx_std_17)

# Should be linked to the main library, as well as the Catch2 testing library
target_link_libraries(rurouni_test PRIVATE rurouni Catch2::Catch2)

target_compile_definitions( rurouni_test PRIVATE
        RR_TESTING )

# If you register a test, then ctest and make test will run it.
# You can also run examples and check the output, as well.
add_test(NAME rurouni_test COMMAND testlib) # Command can be a target
