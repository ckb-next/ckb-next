cmake_minimum_required(VERSION 3.5)
project(ckb-next)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# main projects
add_subdirectory(src/ckb)
add_subdirectory(src/ckb-daemon)

# animation programs
add_subdirectory(src/ckb-gradient)
add_subdirectory(src/ckb-heat)
#add_subdirectory(src/ckb-mviz)
add_subdirectory(src/ckb-pinwheel)
add_subdirectory(src/ckb-rain)
add_subdirectory(src/ckb-random)
add_subdirectory(src/ckb-ripple)
add_subdirectory(src/ckb-wave)