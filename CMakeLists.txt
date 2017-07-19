# TODO rename daemon's plist to follow the rules for naming

# Higher version might be required
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

project(ckb-next)

# Let CMake find custom modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

# Common installation paths as per GNU Coding Standards
include(GNUInstallDirs)

###############################################################################
# Internal booleans
#------------------------------------------------------------------------------
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOS ON CACHE INTERNAL "macOS host")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LINUX ON CACHE INTERNAL "Linux host")
else ()
    message(FATAL_ERROR "${CMAKE_SYSTEM_NAME} is not supported.")
endif ()
###############################################################################


###############################################################################
# Build type
#------------------------------------------------------------------------------
# Set a default build type if none was specified
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif ()
###############################################################################


###############################################################################
# Options
#------------------------------------------------------------------------------
include(CMakeDependentOption)

if (MACOS)
    option( USE_BREW_QT5 "Use Homebrew package for Qt5." ON  )
else ()  #if (LINUX)
    option( WITH_MVIZ    "Build with music visualizer."  OFF )  # Licence violation, close #187 first
endif ()

# GUI
option( WITH_GUI "Build with Qt GUI to manipulate animations." ON  )

# Animations
set(INSTALL_DIR_ANIMATIONS "${CMAKE_INSTALL_LIBEXECDIR}/animations" CACHE STRING "Where to install animations.")
mark_as_advanced(INSTALL_DIR_ANIMATIONS)

option( WITH_ANIMATIONS "Build with animations." ON)
cmake_dependent_option( WITH_GRADIENT  "Build with 'Gradient' animation." ON  "WITH_ANIMATIONS;WITH_GUI" OFF )
cmake_dependent_option( WITH_HEAT      "Build with 'Heat' animation."     OFF "WITH_ANIMATIONS;WITH_GUI" OFF )  # License violation, close #187 first
cmake_dependent_option( WITH_RAIN      "Build with 'Rain' animation."     ON  "WITH_ANIMATIONS;WITH_GUI" OFF )
cmake_dependent_option( WITH_RANDOM    "Build with 'Random' animation."   ON  "WITH_ANIMATIONS;WITH_GUI" OFF )
cmake_dependent_option( WITH_PINWHEEL  "Build with 'Pinwheel' animation." ON  "WITH_ANIMATIONS;WITH_GUI" OFF )
cmake_dependent_option( WITH_RIPPLE    "Build with 'Ripple' animation."   ON  "WITH_ANIMATIONS;WITH_GUI" OFF )
cmake_dependent_option( WITH_WAVE      "Build with 'Wave' animation."     ON  "WITH_ANIMATIONS;WITH_GUI" OFF )

# Compilation flags
option( BASIC_COMPILER_FLAGS    "Basic inspections."  OFF )
option( NORMAL_COMPILER_FLAGS   "Normal inspections." OFF )
option( EXTRA_COMPILER_FLAGS    "Extra inspections."  OFF )

# USB
option ( DEBUG_USB_SEND "Show the contents of USB packets being sent"     OFF )
option ( DEBUG_USB_RECV "Show the contents of USB packets being received" OFF )

# Sanitizers, by arsenm/sanitizers-cmake
option( WITH_SANITIZERS "Enable certain sanitizers for certain targets." OFF )
cmake_dependent_option( SANITIZE_ADDRESS     "Enable AddressSanitizer for sanitized targets."           OFF "WITH_SANITIZERS" OFF )
cmake_dependent_option( SANITIZE_MEMORY      "Enable MemorySanitizer for sanitized targets."            OFF "WITH_SANITIZERS" OFF )
cmake_dependent_option( SANITIZE_THREAD      "Enable ThreadSanitizer for sanitized targets."            OFF "WITH_SANITIZERS" OFF )
cmake_dependent_option( SANITIZE_UNDEFINED   "Enable UndefinedBehaviorSanitizer for sanitized targets." OFF "WITH_SANITIZERS" OFF )
cmake_dependent_option( SANITIZE_LINK_STATIC "Try to link static against sanitizers."                   OFF "WITH_SANITIZERS" OFF )

# Knowledge about generator type is important
# See https://stackoverflow.com/a/24470998/5164080
string(COMPARE EQUAL "${CMAKE_CFG_INTDIR}" "." is_single)
if (is_single)
    message(STATUS "Single-configuration generator")
    # In case of such generator CMAKE_CONFIGURATION_TYPES is empty, therefore:
    list(APPEND CMAKE_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}")
else ()
    message(STATUS "Multi-configuration generator")
endif ()

foreach (build_type "${CMAKE_CONFIGURATION_TYPES}")
    if (${build_type} STREQUAL "Debug")  # For developers
        set(WITH_SANITIZERS         ON)
        set(BASIC_COMPILER_FLAGS    ON)
        set(NORMAL_COMPILER_FLAGS   ON)
        set(EXTRA_COMPILER_FLAGS    ON)
    elseif (${build_type} STREQUAL "RelWithDebInfo")  # For casual builds (default)
        set(BASIC_COMPILER_FLAGS    ON)
        set(NORMAL_COMPILER_FLAGS   ON)
    elseif (${build_type} STREQUAL "MinSizeRel")  # For static blobs
        set(BASIC_COMPILER_FLAGS    ON)
        set(NORMAL_COMPILER_FLAGS   ON)
    elseif (${build_type} STREQUAL "Release")
        set(BASIC_COMPILER_FLAGS    ON)
    else ()
        message(WARNING "Using non-standard build type. Proceed only if you know what you are doing.")
    endif ()
endforeach ()

if (WITH_SANITIZERS)
    find_package(Sanitizers REQUIRED)
endif ()

if (NOT WITH_GUI)
    message(WARNING "Building without GUI. Proceed only if you know what you are doing.")
endif ()
###############################################################################


###############################################################################
# Version
#------------------------------------------------------------------------------
set(${PROJECT_NAME}_VERSION_MAJOR 0)
set(${PROJECT_NAME}_VERSION_MINOR 2)
set(${PROJECT_NAME}_VERSION_PATCH 9)
set(${PROJECT_NAME}_VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}")

# Get version from git, fallback on hardcoded one
include(DetermineVersion)
find_package(Git)
determine_version("${${PROJECT_NAME}_SOURCE_DIR}" "${GIT_EXECUTABLE}" "${PROJECT_NAME}")
set(${PROJECT_NAME}_VERSION "${${PROJECT_NAME}_VERSION}-${CMAKE_SYSTEM_NAME}")
if (NOT ${PROJECT_NAME}_VERSION_IS_RELEASE)
    set(${PROJECT_NAME}_VERSION "${${PROJECT_NAME}_VERSION}-${CMAKE_SYSTEM_VERSION}")
endif ()

set(${CMAKE_PROJECT_NAME}_VERSION "${PROJECT_NAME}_VERSION")
message(STATUS "Version: ${${CMAKE_PROJECT_NAME}_VERSION}")
###############################################################################


###############################################################################
# Locations and paths
#------------------------------------------------------------------------------
if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif ()
###############################################################################


###############################################################################
# Compiler flags
#------------------------------------------------------------------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS ON)

set(BASIC_FLAGS "-Wall -Wno-unused-parameter"
    CACHE STRING "Basic compiler flags (enable BASIC_COMPILER_FLAGS to use)")
set(NORMAL_FLAGS "-Wundef -Wpointer-arith"
    CACHE STRING "Normal compiler flags (enable NORMAL_COMPILER_FLAGS to use)")
set(EXTRA_FLAGS "-Wextra -Wfloat-equal -Wshadow -Wcast-align -Wstrict-prototypes -Wstrict-overflow=5 -Wwrite-strings -Waggregate-return"
    CACHE STRING "Extra compiler flags (enable EXTRA_COMPILER_FLAGS to use)")

mark_as_advanced(BASIC_FLAGS)
mark_as_advanced(NORMAL_FLAGS)
mark_as_advanced(EXTRA_FLAGS)

if (BASIC_COMPILER_FLAGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${BASIC_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${BASIC_FLAGS}")
endif ()

if (NORMAL_COMPILER_FLAGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${NORMAL_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NORMAL_FLAGS}")
endif ()

if (EXTRA_COMPILER_FLAGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_FLAGS}")
endif ()

# GNU recommends -Og with -g while *Clangs have no idea about -Og
if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Og")
endif ()
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")
endif ()
###############################################################################

include(MacroEnsureOutOfSourceBuild)
macro_ensure_out_of_source_build("ckb-next requires an out of source build.\
 Please create a separate build directory and run 'cmake /path/to/ckb-next [options]' there.")

# if (APPLE)
#     set(CMAKE_INSTALL_PREFIX "/Applications")
#     set(MACOSX_BUNDLE_NAME "ckb-next")
#     set(prefix "${MACOSX_BUNDLE_NAME}.app/Contents")
#     set(INSTALL_RUNTIME_DIR "${prefix}/MacOS")
#     set(INSTALL_CMAKE_DIR "${prefix}/Resources")
# endif ()

# set(INSTALL_ANIMATIONS_DIR "${INSTALL_CMAKE_DIR}/ckb-animations")

#include(PrintAllVariables)

add_subdirectory(src)
