# TODO rename daemon's plist to follow the rules for naming

cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

project(ckb-next LANGUAGES C CXX)

set(CMAKE_MODULE_PATH ${ckb-next_SOURCE_DIR}/cmake)

# everything we need to load before proceeding with the main CMake file
include(PreCMakeConfig)

# project options
if (APPLE)
    option(BREW_QT5 "Use Homebrew package for Qt5" ON)
elseif (LINUX)
    option(LINUX_CUSTOM_INSTALL "Manually install project files" OFF)
endif ()
option(WITH_ANIMATIONS "Build and install animations" ON)
option(LOCAL_QUAZIP "Use system's QuaZip library" ON)
option(ENFORCE_QUAZIP "Use only system's QuaZip library without backtracking to hardcoded one" OFF)

set(ckb-next_VERSION_MAJOR 0)  #  |>|>
set(ckb-next_VERSION_MINOR 2)  #  |>|> populated manually, must be updated accordingly to annotated tags
set(ckb-next_VERSION_PATCH 8)  #  |>|>
set(ckb-next_VERSION "${ckb-next_VERSION_MAJOR}.${ckb-next_VERSION_MINOR}.${ckb-next_VERSION_PATCH}")

# enhance the version by a current commit hash if the commit is not a release tag
include(DetermineVersion)
find_package(Git)
determine_version("${ckb-next_SOURCE_DIR}" "${GIT_EXECUTABLE}" "ckb-next")

include(BuildType)
include(BuildLocation)
include(CompilerFlags)

include(MacroEnsureOutOfSourceBuild)
macro_ensure_out_of_source_build("ckb-next requires an out of source build. \
Please create a separate build directory and run 'cmake /path/to/ckb-next [options]' there.")

if (APPLE)
    set(CMAKE_INSTALL_PREFIX "/Applications")
    set(MACOSX_BUNDLE_NAME "ckb-next")
    set(prefix "${MACOSX_BUNDLE_NAME}.app/Contents")
    set(INSTALL_RUNTIME_DIR "${prefix}/MacOS")
    set(INSTALL_CMAKE_DIR "${prefix}/Resources")
elseif (LINUX AND NOT LINUX_CUSTOM_INSTALL)
    set(CMAKE_INSTALL_PREFIX "/opt")
    set(prefix "ckb-next")
    set(INSTALL_RUNTIME_DIR "${prefix}")
    set(INSTALL_CMAKE_DIR "${prefix}")
endif ()

set(INSTALL_ANIMATIONS_DIR "${INSTALL_CMAKE_DIR}/ckb-animations")

add_subdirectory(src)
