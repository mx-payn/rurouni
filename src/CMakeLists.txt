# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
# file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${ModernCMakeExample_SOURCE_DIR}/include/modern/*.hpp")
set(HEADER_LIST "${Rurouni_SOURCE_DIR}/include/Rurouni.hpp")

# Make an automatic library - will be static or dynamic based on user setting
add_library(rurouni Game.cpp ../external/loguru/loguru.cpp ${HEADER_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(rurouni PUBLIC
        ../include/
        ${CMAKE_CURRENT_BINARY_DIR}/
        ${Rurouni_SOURCE_DIR}/external/loguru)

# This depends on (header only) boost
target_link_libraries(rurouni PRIVATE pthread dl )

# Generate export headers
include(GenerateExportHeader)
generate_export_header( rurouni BASE_NAME rr)

# Use precompiled header file
target_precompile_headers( rurouni
        PRIVATE ../include/rr_pch.h )

# All users of this library will need at least C++11
target_compile_features(rurouni PUBLIC cxx_std_11)

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})

###################################################################
###############  COMPILE AND PLATFORM DEFINITIONS  ################
###################################################################
## setting target specific precompiler options/definitions
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # platform is LINUX
    target_compile_definitions( rurouni
            PUBLIC RR_PLATFORM_LINUX )

elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # platform is WINDOWS
    target_compile_definitions( rurouni
            PUBLIC RR_PLATFORM_WINDOWS )

elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # platform is MACOS
    target_compile_definitions( rurouni
            PUBLIC RR_PLATFORM_MACOS )

endif()

## setting single/multi config compiler options/definitions
if(GENERATOR_IS_MULTI_CONFIG)
    # multi config generator like Visual Studio or Xcode
    if(NOT ${CMAKE_CONFIGURATION_TYPES} STREQUAL "Release")
        target_compile_definitions( rurouni
                PUBLIC RR_ENABLE_LOG
                RR_ENABLE_ASSERT
                RR_ENABLE_PROFILE )
    endif()
else()
    # single config generator like make or ninja
    if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
        target_compile_definitions( rurouni
                PUBLIC RR_ENABLE_LOG
                RR_ENABLE_ASSERT
                RR_ENABLE_PROFILE )
    endif()
endif()