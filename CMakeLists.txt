cmake_minimum_required(VERSION 3.5)
project(ckb-next)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)

#version checks
find_package(QT5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)

if (Qt5Widgets_FOUND)
    if (Qt5Widgets_VERSION VERSION_LESS 5.6.0)
        message(FATAL_ERROR "Minimum supported Qt5 version is 5.6!")
    endif()
else()
    message(SEND_ERROR "The Qt5Widgets library could not be found!")
endif(Qt5Widgets_FOUND)



# main projects
add_subdirectory(src/ckb)
add_subdirectory(src/ckb-daemon)

# animation programs
add_subdirectory(src/ckb-gradient)
add_subdirectory(src/ckb-heat)
add_subdirectory(src/ckb-mviz)
add_subdirectory(src/ckb-pinwheel)
add_subdirectory(src/ckb-rain)
add_subdirectory(src/ckb-random)
add_subdirectory(src/ckb-ripple)
add_subdirectory(src/ckb-wave)