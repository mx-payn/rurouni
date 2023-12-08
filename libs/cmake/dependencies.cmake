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
set(GLFW_INSTALL OFF)
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

# msdf-atlas-gen
set(MSDF_ATLAS_BUILD_STANDALONE OFF)
FetchContent_Declare(
  msdf-atlas-gen
  GIT_REPOSITORY https://github.com/Chlumsky/msdf-atlas-gen.git
  GIT_TAG v1.2.2
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
    fmt
    glad 
    glfw 
    glm
    msdf-atlas-gen
    spdlog
    stduuid
    )

# glad library generator
glad_add_library(glad $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:STATIC> API gl:core=4.6)
