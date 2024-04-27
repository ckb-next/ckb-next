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
cmake_minimum_required(VERSION 3.13.4 FATAL_ERROR)

project(ckb-next LANGUAGES C CXX)
include(VERSION.cmake)
# Because cmakedefine does not like dashes...
set(ckb_next_VERSION_IS_RELEASE ${ckb-next_VERSION_IS_RELEASE})

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
set(UDEV_RULE_DIRECTORY "/lib/udev/rules.d" CACHE STRING "Where to install the udev rule.")

if (MACOS)
    option(USE_BREW_QT5 "Use Homebrew package for Qt5." ON)
    option(USE_BREW_QUAZIP "Use Homebrew package for QuaZip." ON)
    option(MAC_LEGACY "Build a legacy version without the karabiner kext." OFF)
    if (MAC_LEGACY)
        set(ckb-next_VERSION "${ckb-next_VERSION}-l")
    endif ()    
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
cmake_dependent_option(WITH_INVADERS "Build with 'Invaders' animation." ON "WITH_ANIMATIONS;WITH_GUI" OFF)
cmake_dependent_option(WITH_LIFE     "Build with 'Life' animation."     ON "WITH_ANIMATIONS;WITH_GUI" OFF)
cmake_dependent_option(WITH_SNAKE    "Build with 'Snake' animation."    ON "WITH_ANIMATIONS;WITH_GUI" OFF)
cmake_dependent_option(WITH_PIPE     "Build with 'Pipe' animation."     ON "WITH_ANIMATIONS;WITH_GUI" OFF)

if (LINUX)
    cmake_dependent_option(WITH_MVIZ  "Build with music visualizer."           ON "WITH_ANIMATIONS;WITH_GUI" OFF)
    cmake_dependent_option(USE_XCB    "Build with features that require XCB."  ON  "WITH_GUI" OFF)
    cmake_dependent_option(USE_DBUS_MENU "Build with DBus support for the tray."  ON  "WITH_GUI" OFF)
    option(USE_PORTAUDIO              "Use portaudio for music visualizer."    OFF)
else ()
    cmake_dependent_option(WITH_MVIZ   "Build with music visualizer."          OFF "WITH_ANIMATIONS;WITH_GUI" OFF)
    set(USE_PORTAUDIO ON)
endif ()
# Options: daemon
option(DISABLE_UPDATER "Disable the ckb-next updater. This option should only be enabled during packaging."    OFF)
option(DEBUG_USB_SEND  "Show the contents of USB packets being sent to device."                                OFF)
option(DEBUG_USB_RECV  "Show the contents of USB packets being received from device through usbrecv()."        OFF)
option(DEBUG_USB_INPUT "Show the contents of USB packets being received from device through the input thread." OFF)
option(DEBUG_MUTEX     "Show debugging information regarding thread synchronisation." OFF)
option(NO_FAIR_MUTEX_QUEUEING "Disable fair mutex queueing. Debugging only." OFF)
option(DEBUG_INPUT_SYNC "Print a debug message every time an event bundle is delivered to the OS." OFF)
option(FPS_COUNTER     "Enable FPS counters." OFF)
option(WERROR     "Enable Werror (for CI)" OFF)

# Make sure NO_FAIR_MUTEX_QUEUEING is set if TSAN is enabled
# Otherwise you end up with threading issues that are not detected
if (SANITIZE_THREAD AND NOT NO_FAIR_MUTEX_QUEUEING)
    message(FATAL_ERROR "Thread Sanitizer was enabled but NO_FAIR_MUTEX_QUEUEING is set to FALSE. Please re-run cmake with -DNO_FAIR_MUTEX_QUEUEING=1 or disable SANITIZE_THREAD.")
elseif (NO_FAIR_MUTEX_QUEUEING AND NOT SANITIZE_THREAD)
    message(WARNING "NO_FAIR_MUTEX_QUEUEING is enabled. Make sure to disable it afterwards.")
endif()

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
    set(INSTALL_DIR_UTILITIES "${CMAKE_INSTALL_LIBEXECDIR}"
        CACHE STRING "Where to install miscellaneous utilities.")
endif ()

# Options: other
option(SAFE_INSTALL "Execute pre-install tasks to ensure correct installation.
    Intended to be used with direct installations without package manager." OFF)
option(SAFE_UNINSTALL "Execute pre-uninstall tasks to ensure correct removal.
    Intended to be used with direct removals without package manager." OFF)
option(PREFER_QT6 "Build with Qt 6 if available." OFF)

set(QT_LIST Qt5)
if (PREFER_QT6)
    list(INSERT QT_LIST 0 Qt6)
endif ()

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

    if (MAC_LEGACY)
        set(PKGPROJ_FILE "ckb-next-legacy.pkgproj")
    else ()
        set(PKGPROJ_FILE "ckb-next.pkgproj")
    endif ()

    add_custom_target(
        macos-package
        COMMAND
          ${CMAKE_MAKE_PROGRAM} -j${cores}
        COMMAND
          sudo ${CMAKE_MAKE_PROGRAM} install
        COMMAND
          packagesbuild ${ckb-next_SOURCE_DIR}/macos/pkgproj/${PKGPROJ_FILE}
        COMMAND
          appdmg ${ckb-next_SOURCE_DIR}/macos/pkgproj/appdmg.json ckb-next_v${ckb-next_VERSION}_${CMAKE_OSX_DEPLOYMENT_TARGET}-SDK.dmg
        WORKING_DIRECTORY
          "${ckb-next_BINARY_DIR}"
        COMMENT
          "Generating macOS package"
        USES_TERMINAL
        VERBATIM)
endif ()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "*** Have SIZEOF void * = 8, so 64-bit")
    set(IS_64_BIT 1)
else ()
    message(STATUS "*** SIZEOF void * != 8, so not 64-bit")
endif ()

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "ckb-next")
set(CPACK_PACKAGE_VENDOR "K DeSouza")
set(CPACK_PACKAGE_CONTACT "ckb-next@googlegroups.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "ckb-next is an open-source driver for Corsair keyboards and mice. \
It aims to bring the features of Corsair's proprietary CUE software to the Linux and Mac operating systems.")
set(CPACK_PACKAGE_FILE_NAME "ckb-next")
set(CPACK_PACKAGE_VERSION_MAJOR "${ckb-next_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${ckb-next_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${ckb-next_VERSION_PATCH}-${ckb-next_VERSION_PATCH_EXTRA}")
set(CPACK_SOURCE_IGNORE_FILES "*.cmake")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/linux/debian/postinst;")
if(IS_64_BIT)
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
else()
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386)
endif()
include(CPack)
