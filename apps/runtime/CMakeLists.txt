cmake_minimum_required(VERSION 3.27)

project(rurouni-runtime
    VERSION 0.0.0
    DESCRIPTION "Game runtime for rurouni projects"
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
add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} rurouni::libs)

target_include_directories(${PROJECT_NAME} 
    PRIVATE ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_BINARY_DIR}/include
    )

