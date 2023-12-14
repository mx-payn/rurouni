include(FetchContent)

# cereal
set(SKIP_PORTABILITY_TEST ON)
set(BUILD_DOC OFF)
set(BUILD_SANDBOX OFF)
set(SKIP_PERFORMANCE_COMPARISON ON)
FetchContent_Declare(
  cereal
  GIT_REPOSITORY https://github.com/USCiLab/cereal.git
  GIT_TAG v1.3.2
  )
# entt
FetchContent_Declare(
  entt
  GIT_REPOSITORY https://github.com/skypjack/entt.git
  GIT_TAG v3.12.2)

# fmt
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.1.1
    )

# glad
# This is the library generator
# call glad_add_library and 
# include ${PROJECT_BINARY_DIR}/gladsources/glad
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.4
    SOURCE_SUBDIR cmake
    )

# glfw
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
    )

# glm
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
    )

# magic_enum
# Static reflection for enums (to string, from string, iteration)
# for modern C++, work with any enum type without any macro or boilerplate code
FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
    GIT_TAG v0.9.5
    )

# spdlog
set(SPDLOG_FMT_EXTERNAL ON)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
    )

# stduuid
FetchContent_Declare(
  stduuid
  GIT_REPOSITORY https://github.com/mariusbancila/stduuid.git
  GIT_TAG v1.2.3
  )

FetchContent_MakeAvailable(
    cereal
    entt
    fmt
    glad 
    glfw 
    glm
    magic_enum
    spdlog
    stduuid
    )

# glad library generator
glad_add_library(glad $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:STATIC> API gl:core=4.6)
