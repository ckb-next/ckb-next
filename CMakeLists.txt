# TODO finish mac support
# TODO finish linux support
# TODO get rid of VERSION and fix qmake to do the above
# TODO search for system's quazip and use it if found

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

