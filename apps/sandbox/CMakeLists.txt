cmake_minimum_required(VERSION 3.27)

project(rurouni-sandbox
    VERSION 0.0.0
    DESCRIPTION "sandbox for testing features"
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

set(RR_SANDBOX_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")

#############################################################
######################  TARGET  #############################
#############################################################
add_executable(${PROJECT_NAME} src/main.cpp src/sandbox.cpp)
add_executable(rurouni::sandbox ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} rurouni::libs)

target_include_directories(${PROJECT_NAME} 
    PRIVATE ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_BINARY_DIR}/include
    )

configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/config.h.cmake"
    "${PROJECT_BINARY_DIR}/include/rurouni/sandbox/config.h"
    @ONLY
    )

