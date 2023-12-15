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

include(GNUInstallDirs)
set(RR_APP_FILES_DIR "${CMAKE_BINARY_DIR}/apps/${PROJECT_NAME}")

set(RR_BIN_DIR "${RR_APP_FILES_DIR}/${CMAKE_INSTALL_BINDIR}")
set(RR_LIB_DIR "${RR_APP_FILES_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(RR_ARCHIVE_DIR "${RR_APP_FILES_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(RR_DATA_DIR "${RR_APP_FILES_DIR}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")
set(RR_CONFIG_DIR "${RR_APP_FILES_DIR}/${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${RR_BIN_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${RR_LIB_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${RR_LIB_DIR})

# the the path to the data directory relative to the executable path
file(RELATIVE_PATH RR_RELATIVE_SANDBOX_DATA_DIR ${RR_BIN_DIR} ${RR_DATA_DIR})
file(RELATIVE_PATH RR_RELATIVE_SANDBOX_CONFIG_DIR ${RR_BIN_DIR} ${RR_CONFIG_DIR})

file(MAKE_DIRECTORY "${RR_APP_FILES_DIR}/${CMAKE_INSTALL_DATADIR}")
file(CREATE_LINK "${CMAKE_CURRENT_SOURCE_DIR}/data" ${RR_DATA_DIR} SYMBOLIC)

message(STATUS "rel data path: ${RR_RELATIVE_SANDBOX_DATA_DIR}")
message(STATUS "rel config path: ${RR_RELATIVE_SANDBOX_CONFIG_DIR}")

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

