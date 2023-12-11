cmake_minimum_required(VERSION 3.27)

project(rurouni-editor
    VERSION 0.0.0
    DESCRIPTION "Editor for rurouni projects"
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
#####################  SCRIPTS  #############################
#############################################################
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(GNUInstallDirs)
include(UserDataDirs)

#############################################################
####################  VARIABLES  ############################
#############################################################
# cxx standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# data paths
# editor needs to know where its data is stored, like icons, fonts etc.
# when building, the data root is inside the build folder
# when installing, the data root is somewhere on the system
# so we need to specify whether we are going to install the target,
# to bake the right path into the binary
if (RR_INSTALL_BUILD)
    ## shared directories
    set(RR_EDITOR_BIN_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")
    set(RR_EDITOR_LIB_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
    set(RR_EDITOR_ARCHIVE_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
    set(RR_EDITOR_SHARED_DATA_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")
    set(RR_EDITOR_SHARED_CONFIG_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}")

    ## user directories
    set(RR_EDITOR_USER_CONFIG_DIR "${USER_CONFIG_DIR}/${PROJECT_NAME}")
    set(RR_EDITOR_USER_DATA_DIR "${USER_DATA_DIR}/${PROJECT_NAME}")
else()
    ## simulating a filesystem inside the build folder
    ## and creating a simlink to the in-source data folder
    file(MAKE_DIRECTORY
        "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_DATADIR}"
        )
    file(CREATE_LINK 
        "${CMAKE_CURRENT_SOURCE_DIR}/data" 
        "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
        SYMBOLIC
    )

    ## shared directories
    set(RR_EDITOR_BIN_DIR "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
    set(RR_EDITOR_LIB_DIR "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
    set(RR_EDITOR_ARCHIVE_DIR "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
    set(RR_EDITOR_SHARED_DATA_DIR "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")
    set(RR_EDITOR_SHARED_CONFIG_DIR "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")

    ## user directories
    ## simulating a filesystem inside the binary dir
    set(RR_EDITOR_USER_CONFIG_DIR "${CMAKE_BINARY_DIR}/user/.config/${PROJECT_NAME}")
    set(RR_EDITOR_USER_DATA_DIR   "${CMAKE_BINARY_DIR}/user/.local/share/${PROJECT_NAME}")
endif()

#############################################################
######################  TARGET  #############################
#############################################################
add_executable(${PROJECT_NAME} src/main.cpp src/editor.cpp)
add_executable(rurouni::editor ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} rurouni::libs)

target_include_directories(${PROJECT_NAME} 
    PRIVATE ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_BINARY_DIR}/include
    )

configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/config.h.cmake"
    "${PROJECT_BINARY_DIR}/include/rurouni/editor/config.h"
    @ONLY
    )

