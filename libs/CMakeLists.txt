cmake_minimum_required(VERSION 3.27)

project(
    rurouni-libs
    VERSION 0.0.0
    DESCRIPTION "Interface for all rurouni libraries"
    )

#############################################################
####################  VARIABLES  ############################
#############################################################
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#############################################################
###################  DEPENDENCIES  ##########################
#############################################################
include(cmake/dependencies.cmake)

#############################################################
####################  LIBRARIES  ############################
#############################################################
# order matters!
# the libraries are dependent downwards.
# NOTE: maybe make each library add_subdirectory its dependent
#       libraries, to make decoupling easier?
add_subdirectory(pch)
add_subdirectory(common)

#############################################################
####################  INTERFACE  ############################
#############################################################
add_library(${PROJECT_NAME} INTERFACE)
add_library(rurouni::libs ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} 
    INTERFACE   rurouni::common
    )

