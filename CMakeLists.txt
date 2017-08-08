#
#        Copyright © 2017 Oleksii Vilchanskyi <alexvilchansky@yahoo.com>
#        
#        This file is part of ckb-next.
#
#        ckb-next is free software: you can redistribute it and/or modify
#        it under the terms of the GNU General Public License version 2
#        as published by the Free Software Foundation.
#
#        ckb-next is distributed in the hope that it will be useful,
#        but WITHOUT ANY WARRANTY; without even the implied warranty of
#        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#        GNU General Public License version 2 for more details.
#
#        You should have received a copy of the GNU General Public License
#        version 2 along with ckb-next. If not, see
#        <https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
#

# TODO: write up a new doc for pkg generation

# NOTE: Intended for macOS pkg generation, see docs under macos/
# set(CMAKE_OSX_DEPLOYMENT_TARGET 10.10)

# Change this to lower _only_ if tested to work correctly with that version!
# Might be as low as 3.3, but again, use at your own risk
cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

# However, never downgrade this. It indicates listfiles were written for 3.8
cmake_policy(VERSION 3.8)

project(ckb-next LANGUAGES C CXX VERSION 0.2.9) # bumb this in release commit

# Let CMake find custom modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

include(MacroEnsureOutOfSourceBuild)
macro_ensure_out_of_source_build("ckb-next requires an out of source build. \
Please create a separate build directory and run 'cmake /path/to/ckb-next [options]' there.")

# Set platform-specific booleans
include(CkbNextPlatform)

# Set a default build type if none was specified
include(CkbNextBuildType)

# Get more precise version from git, fallback on release
include(CkbNextDetermineVersion)
find_package(Git)
determine_version("${ckb-next_SOURCE_DIR}" "${GIT_EXECUTABLE}" "ckb-next")

# Options for the whole project follow

include(CMakeDependentOption)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE STRING "Where to put binaries after compilation.")

if (MACOS)
    option(USE_BREW_QT5 "Use Homebrew package for Qt5." ON)
elseif (LINUX)
    option(WITH_MVIZ "Build with music visualizer." ON)
endif ()

# GUI
option(WITH_GUI "Build with Qt GUI to manipulate animations." ON)

# GUI animations
option(WITH_ANIMATIONS "Build with animations." ON)
cmake_dependent_option(WITH_GRADIENT "Build with 'Gradient' animation." ON "WITH_ANIMATIONS;WITH_GUI" OFF)
cmake_dependent_option(WITH_HEAT     "Build with 'Heat' animation."     ON "WITH_ANIMATIONS;WITH_GUI" OFF)
cmake_dependent_option(WITH_RAIN     "Build with 'Rain' animation."     ON "WITH_ANIMATIONS;WITH_GUI" OFF)
cmake_dependent_option(WITH_RANDOM   "Build with 'Random' animation."   ON "WITH_ANIMATIONS;WITH_GUI" OFF)
cmake_dependent_option(WITH_PINWHEEL "Build with 'Pinwheel' animation." ON "WITH_ANIMATIONS;WITH_GUI" OFF)
cmake_dependent_option(WITH_RIPPLE   "Build with 'Ripple' animation."   ON "WITH_ANIMATIONS;WITH_GUI" OFF)
cmake_dependent_option(WITH_WAVE     "Build with 'Wave' animation."     ON "WITH_ANIMATIONS;WITH_GUI" OFF)

# Daemon's USB protocol
option(DEBUG_USB_SEND "Show the contents of USB packets being sent to device."       OFF)
option(DEBUG_USB_RECV "Show the contents of USB packets being received from device." OFF)

# Sanitizers
find_package(Sanitizers)
option(WITH_SANITIZERS "Enable certain sanitizers for certain targets. \
By default ASan and UBSan will be enabled with this option." OFF)
cmake_dependent_option(SANITIZE_ADDRESS     "Enable AddressSanitizer."               ON  "WITH_SANITIZERS" OFF)
cmake_dependent_option(SANITIZE_UNDEFINED   "Enable UndefinedBehaviorSanitizer."     ON  "WITH_SANITIZERS" OFF)
cmake_dependent_option(SANITIZE_MEMORY      "Enable MemorySanitizer."                OFF "WITH_SANITIZERS" OFF)
cmake_dependent_option(SANITIZE_THREAD      "Enable ThreadSanitizer."                OFF "WITH_SANITIZERS" OFF)
cmake_dependent_option(SANITIZE_LINK_STATIC "Try linking static against sanitizers." OFF "WITH_SANITIZERS" OFF)

# Install
include(GNUInstallDirs)
set(INSTALL_DIR_ANIMATIONS "${CMAKE_INSTALL_LIBEXECDIR}/ckb-animations" CACHE STRING "Where to install animations.")

option(SAFE_INSTALL "Execute pre-install tasks to ensure correct installation. \
Intended to be used with direct installations without package manager." ON)

if (NOT WITH_GUI)
    message(WARNING "Building without GUI.")
endif ()

# Project-specific compiler settings
include(CkbNextCompileFlags)

add_subdirectory(src)
