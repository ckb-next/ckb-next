cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(ckb-next LANGUAGES C CXX)

set(CMAKE_MODULE_PATH ${ckb-next_SOURCE_DIR}/cmake)

# everything we need to load before proceeding with the main CMake file
include(PreCMakeConfig)

set(ckb-next_VERSION_MAJOR "0")  #  |>|>
set(ckb-next_VERSION_MINOR "2")  #  |>|> populated manually, must be updated accordingly to release tags
set(ckb-next_VERSION_PATCH "7")  #  |>|>
set(ckb-next_VERSION "${ckb-next_VERSION_MAJOR}.${ckb-next_VERSION_MINOR}.${ckb-next_VERSION_PATCH}")

# enhance the version by a current commit hash if the commit is not a release tag
include(DetermineVersion)
find_package(Git)
determine_version(${ckb-next_SOURCE_DIR} ${GIT_EXECUTABLE} "ckb-next")

# set build type (default one is relwithdebinfo)
include(BuildType)

# set up binary paths
include(BuildLocation)

# set up compiler flags
include(CompilerFlags)

# disallow in-source build
include(MacroEnsureOutOfSourceBuild)
macro_ensure_out_of_source_build("ckb-next requires an out of source build. \
Please create a separate build directory and run 'cmake /path/to/ckb-next [options]' there.")


#if (APPLE)
#    set(MACOSX_BUNDLE_NAME "CKB Next")
#    set(prefix "${MACOSX_BUNDLE_NAME}.app/Contents")
#    set(INSTALL_INCLUDE_DIR "${prefix}/${INSTALL_INCLUDE_DIR}")
#    set(INSTALL_RUNTIME_DIR "${prefix}/MacOS")
#    set(INSTALL_LIBRARY_DIR "${prefix}/${INSTALL_LIBRARY_DIR}")
#    set(INSTALL_ARCHIVE_DIR "${prefix}/${INSTALL_ARCHIVE_DIR}")
#    set(INSTALL_DATA_DIR "${prefix}/${INSTALL_DATA_DIR}")
#    set(INSTALL_DOC_DIR "${prefix}/${INSTALL_DOC_DIR}")
#    set(INSTALL_CMAKE_DIR "${prefix}/Resources")
#endif ()

# proceed with source files
add_subdirectory(src)

# and a bundle
include(CkbNextBundle)
# and a dmg
include(CkbNextCPack)

include(CPack)


#
## create a cosy boolean for Linux
#if (UNIX AND NOT APPLE)
#    set(LINUX TRUE)
#else ()
#    set(LINUX FALSE)
#endif ()
#
## get version string from VERSION
#execute_process(COMMAND cat ${CMAKE_SOURCE_DIR}/VERSION RESULT_VARIABLE CKB_VERSION_STR)
#add_definitions(-DCKB_VERSION_STR="\\\"${CKB_VERSION_STR}\\\"")
#
#message(STATUS "PROJECT: ${PROJECT_NAME} ${CKB_VERSION_STR}")
#
## force build type
##if (NOT CMAKE_BUILD_TYPE)
##    set(CMAKE_BUILD_TYPE Release CACHE STRING  # TODO change this after cmake is fully functional
##    "None Debug Release RelWithDebInfo MinSizeRel"
##    FORCE)
##endif ()
##message(STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
#
## where binaries/bundles go
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
#
## where cmake-related files live
#set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
#
## disallow in-source build
#include(MacroEnsureOutOfSourceBuild)
#macro_ensure_out_of_source_build("${PROJECT_NAME} requires an out of source build. \
#Please create a separate build directory and run 'cmake /path/to/${PROJECT_NAME} [options]' there.")
#
#if (APPLE)
#    message(STATUS "Building for macOS")
#else ()
#    message(STATUS "Building for Linux")
#endif ()
#
## proceed with source files
#add_subdirectory(src)