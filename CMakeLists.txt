# TODO rename daemon's plist to follow the rules for naming

# Higher version might be required
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

project(ckb-next)

# Let CMake find custom modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

###############################################################################
# Internal booleans
#------------------------------------------------------------------------------
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOS TRUE CACHE INTERNAL "macOS host" FORCE)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LINUX TRUE CACHE INTERNAL "Linux host" FORCE)
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
if (MACOS AND NOT LINUX)
    option(  BREW_QT5                     "Use Homebrew package for Qt5"                  ON  )
elseif (LINUX AND NOT MACOS)
    option(  WITH_MVIZ                    "Build with music visualizer"                   OFF )      # Licence violation, close #187 first
endif ()

if (MACOS OR LINUX)
    option(  WITH_SANITIZERS              "Run some sanitizers on some targets"           OFF )
    option(  WITH_GUI                     "Build with Qt GUI to manipulate animations"    ON  )
    # Animations
    option(  WITH_GRADIENT                "Build with 'Gradient' animation"               ON  )
    option(  WITH_HEAT                    "Build with 'Heat' animation"                   OFF )      # License violation, close #187 first
    option(  WITH_PINWHEEL                "Build with 'Pinwheel' animation"               ON  )
    option(  WITH_RAIN                    "Build with 'Rain' animation"                   ON  )
    option(  WITH_RANDOM                  "Build with 'Random' animation"                 ON  )
    option(  WITH_RIPPLE                  "Build with 'Ripple' animation"                 ON  )
    option(  WITH_WAVE                    "Build with 'Wave' animation"                   ON  )
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")                                                               # For developers
    option(  BASIC_COMPILER_FLAGS         "Basic inspections"                             ON  )
    option(  NORMAL_COMPILER_FLAGS        "Normal inspections"                            ON  )
    option(  EXTRA_COMPILER_FLAGS         "Extra inspections"                             ON  )
    option(  WITH_SANITIZERS              "Run some sanitizers on some targets"           ON  )
elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")                                                  # For casual builds (default)
    option(  BASIC_COMPILER_FLAGS         "Basic inspections"                             ON  )
    option(  NORMAL_COMPILER_FLAGS        "Severe inspections"                            ON  )
    option(  EXTRA_COMPILER_FLAGS         "Extra inspections"                             OFF )
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    option(  BASIC_COMPILER_FLAGS         "Basic inspections"                             ON  )
    option(  NORMAL_COMPILER_FLAGS        "Severe inspections"                            OFF )
    option(  EXTRA_COMPILER_FLAGS         "Extra inspections"                             OFF )
elseif (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")                                                      # For static blobs
    option(  BASIC_COMPILER_FLAGS         "Basic inspections"                             ON  )
    option(  NORMAL_COMPILER_FLAGS        "Severe inspections"                            ON  )
    option(  EXTRA_COMPILER_FLAGS         "Extra inspections"                             OFF )
else ()
    message(FATAL_ERROR "Blame CMake developers")
endif ()

if (WITH_SANITIZERS)
    find_package(Sanitizers REQUIRED)
    # Options to enable sanitizers will appear in CMake GUI
    # They are:
    # SANITIZE_ADDRESS, SANITIZE_MEMORY, SANITIZE_THREAD or SANITIZE_UNDEFINED
    # SANITIZE_LINK_STATIC to link sanitizers for gcc static
endif ()

if (NOT WITH_GUI)
    message(WARNING "Building without GUI is NOT recommended. Proceed only if you know what you are doing.")
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
message(STATUS "Version: ${${PROJECT_NAME}_VERSION}")
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
# TODO: propagate -Werror into the project
#------------------------------------------------------------------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS ON)

# FIXME: suppress daemon dummies with #pragma or reimplement
set(BASIC_FLAGS "-Wall -Wno-unused-parameter" CACHE STRING "Basic compiler flags (enable BASIC_COMPILER_FLAGS to use)")
set(NORMAL_FLAGS "-Wundef -Wpointer-arith" CACHE STRING "Normal compiler flags (enable NORMAL_COMPILER_FLAGS to use)")
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

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # GNU recommends -Og with -g while *Clangs have no idea about -Og
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Og")
    endif ()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")
    endif ()
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
