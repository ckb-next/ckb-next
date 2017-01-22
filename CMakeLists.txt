# TODO finish mac support
# TODO finish linux support
# TODO check build type before defining version, if release - use manually passed string with -D, else use git tags
# TODO get rid of VERSION and fix qmake to do the above
# TODO add option to enable/disable compiler warnings
# TODO add option to build or not build the gui
# TODO add option to build or not build the animations

cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

project(ckb-next)

set(CMAKE_MODULE_PATH ${${PROJECT_NAME}_SOURCE_DIR}/cmake)

set(${PROJECT_NAME}_VERSION_MAJOR   "0")
set(${PROJECT_NAME}_VERSION_MINOR   "2")
set(${PROJECT_NAME}_VERSION_PATCH   "7")
set(${PROJECT_NAME}_VERSION
        "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}")

include(DetermineVersion)
find_package(Git)
determine_version(${${PROJECT_NAME}_SOURCE_DIR} ${GIT_EXECUTABLE} "${PROJECT_NAME}")

# define version for source code
add_definitions(-DCKB_VERSION_STR="\\\"${${PROJECT_NAME}_VERSION}\\\"")

# set build type (debug, release etc.)
include(BuildType)
message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

# set up binaries/libraries paths
include(BuildLocation)

# set up compiler flags
include(CompilerFlags)

# disallow in-source build
include(MacroEnsureOutOfSourceBuild)
macro_ensure_out_of_source_build("${PROJECT_NAME} requires an out of source build. \
Please create a separate build directory and run 'cmake /path/to/${PROJECT_NAME} [options]' there.")

# create a boolean for Linux
if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
else ()
    set(LINUX FALSE)
endif ()

# proceed with source files
add_subdirectory(src)
