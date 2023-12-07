include(FetchContent)

# fmt
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.1.1
    )

# spdlog
set(SPDLOG_FMT_EXTERNAL ON)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
    )

FetchContent_MakeAvailable(
    fmt
    spdlog
    )
