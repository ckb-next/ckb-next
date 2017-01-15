project(ckb)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "-lz -Wno-unused-parameter")
set(CMAKE_C_FLAGS "-Wno-unused-parameter")

set(CMAKE_CXX_FLAGS_DEBUG "")
set(CMAKE_C_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_RELEASE "")
set(CMAKE_C_FLAGS_RELEASE "")


# Qt automagic things
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Qt libs
#find_package(Qt5Core)
#find_package(Qt5Widgets)

#version checks
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

if (Qt5Widgets_FOUND)
    if (Qt5Widgets_VERSION VERSION_LESS 5.2.0)
        message(FATAL_ERROR "Minimum supported Qt5 version is 5.2!")
    endif()
else()
    message(SEND_ERROR "The Qt5Widgets library could not be found!")
endif(Qt5Widgets_FOUND)

file(GLOB HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/quazip/*.h")

file(GLOB SOURCE
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/quazip/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/quazip/*.c")

file(GLOB FORMS "${CMAKE_CURRENT_SOURCE_DIR}/*.ui")
file(GLOB RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.qrc")


set(CHECK_X11 "pkg-config --exists x11")
set(APP_IND "pkg-config --exists appindicator-0.1")

execute_process(COMMAND ${CHECK_X11} RESULT_VARIABLE RESULT)
if ("${RESULT}" STREQUAL "0")
    message("Using X11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lX11")
    add_definitions(-DUSE_LIBX11)
endif("${RESULT}" STREQUAL "0")

# todo: set this up.
execute_process(COMMAND ${APP_IND} RESULT_VARIABLE RESULT)
if ("${RESULT}" STREQUAL "0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lX11")
    add_definitions(-DUSE_LIBX11)
endif("${RESULT}" STREQUAL "0")

execute_process(COMMAND cat ${CMAKE_SOURCE_DIR}/VERSION RESULT_VARIABLE CKB_VERSION_STR)
add_definitions(-DCKB_VERSION_STR="\\\"${CKB_VERSION_STR}\\\"")

set(TARGET_LIBS
        Qt5::Core
        Qt5::Widgets
        Qt5::Network)

add_executable(${PROJECT_NAME} ${SOURCE} ${HEADER})
target_link_libraries(${PROJECT_NAME} ${TARGET_LIBS})