cmake_minimum_required(VERSION 3.27)

project(
    rurouni
    DESCRIPTION "A 2D roguelike game and editor for educational games"
    )

#############################################################
######################  OPTIONS  ############################
#############################################################
option(RR_INSTALL_BUILD "use the install dir paths instead of build paths for data, assets, output... dirs" OFF)

#############################################################
####################  VARIABLES  ############################
#############################################################
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#############################################################
####################  SUBDIRS  ##############################
#############################################################

# interface to rurouni libraries
add_subdirectory(libs ${CMAKE_BINARY_DIR}/rurouni-libs)
add_subdirectory(apps/runtime ${CMAKE_BINARY_DIR}/rurouni-runtime)
add_subdirectory(apps/editor ${CMAKE_BINARY_DIR}/rurouni-editor)
add_subdirectory(apps/sandbox ${CMAKE_BINARY_DIR}/rurouni-sandbox)
