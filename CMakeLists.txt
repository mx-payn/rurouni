cmake_minimum_required(VERSION 3.17)

# Project name and a few useful settings. Other commands can pick up the results
project(
  Rurouni
  VERSION 0.0.0
  DESCRIPTION "A 2D ASCII game-engine for educational games on programming languages."
  HOMEPAGE_URL "https://mx-payn.github.io/rurouni/"
  LANGUAGES CXX)

# exporting compile commands for auto suggestion/completion features
# produces a compile-commands.json with all compile option information
# e.g. for emacs
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

# should we build static libs?
set(RUROUNI_SHARED YES)
if (RUROUNI_SHARED)
  set(RUROUNI_LIB_TYPE SHARED)
else ()
  set(RUROUNI_LIB_TYPE STATIC)
endif ()

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

  # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)

  # Let's ensure -std=c++xx instead of -std=g++xx
  set(CMAKE_CXX_EXTENSIONS OFF)

  # Let's nicely support folders in IDEs
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  ## setting custom output directories outside the current target subdir to find shared libraries easier
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

  # change default visibility of exposed classes and functions to hidden
  # only relevant for SHARED lib builds. This is default on Windows, but not on UNIX systems
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  # change visibility of inlined code (including templates) to hidden
  set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

  # Testing only available if this is the main app
  # Note this needs to be done in the main CMakeLists
  # since it calls enable_testing, which must be in the
  # main CMakeLists.
  include(CTest)

  # Docs only available if this is the main app
  find_package(Doxygen)
  if(Doxygen_FOUND)
    add_subdirectory(docs)
  else()
    message(STATUS "Doxygen not found, not building docs")
  endif()
endif()

# FetchContent added in CMake 3.11, downloads during the configure step
include(FetchContent)
# FetchContent_MakeAvailable was not added until CMake 3.14; use our shim
if(${CMAKE_VERSION} VERSION_LESS 3.14)
  include(cmake/add_FetchContent_MakeAvailable.cmake)
endif()

###################################################################
########################  SUBDIRECTORIES  #########################
###################################################################
# The compiled library code is here
add_subdirectory(src)

# The executable code is here
add_subdirectory(apps/sandbox)

# Testing only available if this is the main app
# Emergency override MODERN_CMAKE_BUILD_TESTING provided as well
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING)
   AND BUILD_TESTING)
  add_subdirectory(tests)
endif()
