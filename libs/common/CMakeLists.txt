cmake_minimum_required(VERSION 3.27)

project(rurouni-common
    VERSION 0.0.0
    DESCRIPTION "rurouni engine common library"
    LANGUAGES CXX
    )

# include guard
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
    "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

#############################################################
####################  VARIABLES  ############################
#############################################################
# cxx standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#############################################################
######################  TARGET  #############################
#############################################################
add_library(${PROJECT_NAME} STATIC 
    # collections
    src/math.cpp
    # dev
    src/dev/logger.cpp 
    # event
    src/event/event_system.cpp
    # system
    src/system/filesystem.cpp
    # types
    src/types/uuid.cpp
)
add_library(rurouni::common ALIAS ${PROJECT_NAME})

target_include_directories(
    ${PROJECT_NAME} 
    PUBLIC  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
)

target_link_libraries(${PROJECT_NAME} 
    PUBLIC  cereal::cereal
            fmt::fmt
            glm::glm
            spdlog::spdlog 
            stduuid
    PRIVATE rurouni::pch
)

target_precompile_headers(${PROJECT_NAME} REUSE_FROM rurouni::pch)

#############################################################
#####################  GENERATE  ############################
#############################################################
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/config.h.cmake"
    "${PROJECT_BINARY_DIR}/include/rurouni/common/config.h"
    @ONLY
)

