cmake_minimum_required(VERSION 3.7)

project(ckb-next)
message(STATUS "PROJECT_NAME: " ${PROJECT_NAME})

# define version
execute_process(COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/VERSION
        OUTPUT_VARIABLE CKB_VERSION_STR OUTPUT_STRIP_TRAILING_WHITESPACE)
add_definitions(-DCKB_VERSION_STR="\\\"${CKB_VERSION_STR}\\\"")
#add_definitions(-DCKB_VERSION_STR=${${PROJECT_NAME}_VERSION})
message(STATUS "CKB_VERSION_STR: " ${CKB_VERSION_STR})

# choose build type
message(STATUS "CMAKE_BUILD_TYPE before forcing: " ${CMAKE_BUILD_TYPE})
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
            "None Debug Release RelWithDebInfo MinSizeRel"
            FORCE)
endif()
message(STATUS "CMAKE_BUILD_TYPE after forcing: " ${CMAKE_BUILD_TYPE})

# where binaries/bundles go
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
message(STATUS "CMAKE_BINARY_DIR: " ${CMAKE_BINARY_DIR})

# where cmake-related files live
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

add_subdirectory(src)
