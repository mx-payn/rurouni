cmake_minimum_required(VERSION 3.27)

project(
    rurouni
    DESCRIPTION "A 2D roguelike game and editor for educational games"
    )

#############################################################
####################  VARIABLES  ############################
#############################################################
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)
# output paths
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})

#############################################################
####################  SUBDIRS  ##############################
#############################################################

# interface to rurouni libraries
add_subdirectory(libs ${CMAKE_BINARY_DIR}/rurouni-libs)
