#   Copyright 2017-2018 ckb-next Development Team <ckb-next@googlegroups.com>
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions are met:
#   
#   1. Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
#   2. Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#   3. Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission. 
#   
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#   POSSIBILITY OF SUCH DAMAGE.

# Lower it _only_ if tested to work correctly with that version!
# Might be as low as 3.3, but again, use at your own risk
cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)

project(ckb-next LANGUAGES C CXX)
include(VERSION.cmake)

# Let CMake find custom modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules/sanitizers")

include(MacroEnsureOutOfSourceBuild)
macro_ensure_out_of_source_build("ckb-next requires an out of source build. \
Please create a separate build directory and run 'cmake /path/to/ckb-next [options]' there.")

# Make sure we don't install in anaconda
if("$ENV{PATH}" MATCHES "anaconda")
    string(REPLACE ":" ";" ENVLIST "$ENV{PATH}")

    foreach(item ${ENVLIST})
        if("${item}" MATCHES "anaconda")
            list(REMOVE_ITEM ENVLIST "${item}")
        endif()
    endforeach()

    string(REPLACE ";" ":" ENVSTR "${ENVLIST}")
    set(ENV{PATH} ${ENVSTR})
    message(STATUS "Temporarily removing anaconda from PATH:\n   $ENV{PATH}")
endif()

# Set platform-specific booleans
include(CkbNextPlatform)

# Set a default build type if none was specified
include(CkbNextBuildType)

# Get more precise version from git, fallback on release
include(CkbNextDetermineVersion)
find_package(Git)
determine_version("${ckb-next_SOURCE_DIR}" "${GIT_EXECUTABLE}" "ckb-next")

find_package(Sanitizers)

# Options for the whole project follow

include(CMakeDependentOption)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE STRING "Where to put binaries after compilation.")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE STRING "Where to put libraries after compilation.")

if (MACOS)
    option(USE_BREW_QT5 "Use Homebrew package for Qt5." ON)
    option(USE_BREW_QUAZIP "Use Homebrew package for QuaZip." ON)
elseif (LINUX)
    option(WITH_MVIZ "Build with music visualizer." ON)
endif ()

# Options: GUI
option(WITH_GUI "Build with Qt GUI to manipulate animations." ON)

# Options: GUI animations
option(WITH_ANIMATIONS "Build with animations." ON)
cmake_dependent_option(WITH_GRADIENT "Build with 'Gradient' animation." ON "WITH_ANIMATIONS;WITH_GUI" OFF)
cmake_dependent_option(WITH_HEAT     "Build with 'Heat map' animation." ON "WITH_ANIMATIONS;WITH_GUI" OFF)
cmake_dependent_option(WITH_RAIN     "Build with 'Rain' animation."     ON "WITH_ANIMATIONS;WITH_GUI" OFF)
cmake_dependent_option(WITH_RANDOM   "Build with 'Random' animation."   ON "WITH_ANIMATIONS;WITH_GUI" OFF)
cmake_dependent_option(WITH_PINWHEEL "Build with 'Pinwheel' animation." ON "WITH_ANIMATIONS;WITH_GUI" OFF)
cmake_dependent_option(WITH_RIPPLE   "Build with 'Ripple' animation."   ON "WITH_ANIMATIONS;WITH_GUI" OFF)
cmake_dependent_option(WITH_WAVE     "Build with 'Wave' animation."     ON "WITH_ANIMATIONS;WITH_GUI" OFF)

# Options: daemon
option(DEBUG_USB_SEND  "Show the contents of USB packets being sent to device."                                OFF)
option(DEBUG_USB_RECV  "Show the contents of USB packets being received from device through usbrecv()."        OFF)
option(DEBUG_USB_INPUT "Show the contents of USB packets being received from device through the input thread." OFF)

# Options: install
if (MACOS)
    set(CMAKE_INSTALL_PREFIX "/Applications")
endif ()

include(GNUInstallDirs)
set(ANIMATIONS_DIR_NAME "ckb-next-animations")
if (MACOS)
    set(INSTALL_DIR_ANIMATIONS "ckb-next.app/Contents/Resources/${ANIMATIONS_DIR_NAME}"
        CACHE STRING "Where to install animations.")
elseif (LINUX)
    set(INSTALL_DIR_ANIMATIONS "${CMAKE_INSTALL_LIBEXECDIR}/${ANIMATIONS_DIR_NAME}"
        CACHE STRING "Where to install animations.")
endif ()

# Options: other
option(SAFE_INSTALL "Execute pre-install tasks to ensure correct installation.
    Intended to be used with direct installations without package manager." OFF)
option(SAFE_UNINSTALL "Execute pre-uninstall tasks to ensure correct removal.
    Intended to be used with direct removals without package manager." OFF)

option(WITH_SHIPPED_QUAZIP "Use shipped QuaZip5 instead of system package" OFF)
option(USE_APPINDICATOR_OVERRIDE "Use gtk2 Appindicator for Qt > 5.7." OFF)

if (NOT WITH_GUI)
    message(WARNING "Building without GUI. Proceed only if you know what you are doing.")
endif ()

# Project-specific compiler settings
include(CkbNextCompileFlags)

add_subdirectory(src)

# Uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    @ONLY)

add_custom_target(
    uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    USES_TERMINAL)

# macOS packaging
if (MACOS AND WITH_GUI)
    include(ProcessorCount)
    ProcessorCount(cores)
    if ("${cores}" EQUAL 0)
        message(FATAL_ERROR "Counted 0 cores")
    else ()
        message(STATUS "Number of cores: ${cores}")
    endif ()

    add_custom_target(
        macos-package
        COMMAND
          ${CMAKE_MAKE_PROGRAM} -j${cores}
        COMMAND
          sudo ${CMAKE_MAKE_PROGRAM} install
        COMMAND
          packagesbuild ${ckb-next_SOURCE_DIR}/macos/pkgproj/ckb-next.pkgproj
        COMMAND
          appdmg ${ckb-next_SOURCE_DIR}/macos/pkgproj/appdmg.json ckb-next_v${ckb-next_VERSION}_${CMAKE_OSX_DEPLOYMENT_TARGET}-SDK.dmg
        WORKING_DIRECTORY
          "${ckb-next_BINARY_DIR}"
        COMMENT
          "Generating macOS package"
        USES_TERMINAL
        VERBATIM)
endif ()
