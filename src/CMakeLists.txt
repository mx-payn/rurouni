# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
# file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${ModernCMakeExample_SOURCE_DIR}/include/modern/*.hpp")
set(HEADER_LIST "${Rurouni_SOURCE_DIR}/include/Rurouni.hpp")

# Make an automatic library - will be static or dynamic based on user setting
add_library(rurouni Game.cpp ${HEADER_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(rurouni PUBLIC
        ../include/
        ${CMAKE_CURRENT_BINARY_DIR}/ )

# This depends on (header only) boost
#target_link_libraries(rurouni PRIVATE Boost::boost)

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
    target_compile_definitions( rurouni
            PUBLIC RR_PLATFORM_LINUX )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions( rurouni
            PUBLIC RR_PLATFORM_WINDOWS )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions( rurouni
            PUBLIC RR_PLATFORM_MAC )
endif()