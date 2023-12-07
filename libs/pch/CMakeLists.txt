cmake_minimum_required(VERSION 3.27)

project(
    rurouni-pch
    VERSION 0.0.0
    DESCRIPTION "Precompiled headers for rurouni libraries"
    )

#############################################################
######################  TARGET  #############################
#############################################################
add_library(${PROJECT_NAME} OBJECT ${CMAKE_CURRENT_BINARY_DIR}/src/pch.cpp)
add_library(rurouni::pch ALIAS ${PROJECT_NAME})

target_precompile_headers(${PROJECT_NAME} 
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/rurouni/pch.hpp
    )

target_include_directories(${PROJECT_NAME} 
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
    )

#############################################################
#####################  GENERATE  ############################
#############################################################
# cmake needs to know the language to compile the OBJECT target, so
# a dummy cpp file is needed
file(GENERATE
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/src/pch.cpp
  CONTENT ""
)
