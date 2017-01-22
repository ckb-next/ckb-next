cmake_minimum_required(VERSION 3.5)
project(ckb-next)

# create a cosy boolean for Linux
if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
else ()
    set(LINUX FALSE)
endif ()

# get version string from VERSION
execute_process(COMMAND cat ${CMAKE_SOURCE_DIR}/VERSION RESULT_VARIABLE CKB_VERSION_STR)
add_definitions(-DCKB_VERSION_STR="\\\"${CKB_VERSION_STR}\\\"")

message(STATUS "PROJECT: ${PROJECT_NAME} ${CKB_VERSION_STR}")

# force build type
#if (NOT CMAKE_BUILD_TYPE)
#    set(CMAKE_BUILD_TYPE Release CACHE STRING  # TODO change this after cmake is fully functional
#    "None Debug Release RelWithDebInfo MinSizeRel"
#    FORCE)
#endif ()
#message(STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})

# where binaries/bundles go
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# where cmake-related files live
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# disallow in-source build
include(MacroEnsureOutOfSourceBuild)
macro_ensure_out_of_source_build("${PROJECT_NAME} requires an out of source build. \
Please create a separate build directory and run 'cmake /path/to/${PROJECT_NAME} [options]' there.")

if (APPLE)
    message(STATUS "Building for macOS")
else ()
    message(STATUS "Building for Linux")
endif ()

# proceed with source files
add_subdirectory(src)