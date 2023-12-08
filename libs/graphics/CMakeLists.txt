cmake_minimum_required(VERSION 3.27)

project(rurouni-graphics
    VERSION 0.0.0
    DESCRIPTION "rurouni graphics library"
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
add_library(${PROJECT_NAME} 
    src/window.cpp 
    src/render_api.cpp
    src/index_buffer.cpp
    src/vertex_buffer.cpp
    src/vertex_array.cpp
    src/shader.cpp
    src/framebuffer.cpp
    src/texture.cpp
    src/batch_renderer.cpp
    src/debug_renderer.cpp
    src/camera.cpp
    src/font.cpp
    src/text.cpp
    )
add_library(rurouni::graphics ALIAS ${PROJECT_NAME})

target_include_directories(
    ${PROJECT_NAME} 
    PUBLIC  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    PRIVATE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/gladsources/glad/include>
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)

target_link_libraries(${PROJECT_NAME} 
    PUBLIC  rurouni::common
    PRIVATE rurouni::pch
            glfw
            glad
            msdf-atlas-gen::msdf-atlas-gen
)

target_precompile_headers(${PROJECT_NAME} REUSE_FROM rurouni::pch)

#############################################################
#####################  GENERATE  ############################
#############################################################
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/config.h.cmake"
    "${PROJECT_BINARY_DIR}/include/rurouni/graphics/config.h"
    @ONLY
)

