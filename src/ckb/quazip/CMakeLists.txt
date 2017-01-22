project(quazip)

set(CMAKE_CXX_FLAGS "-Wno-unused-parameter")
set(CMAKE_C_FLAGS "-Wno-unused-parameter")

find_package(Qt5Core REQUIRED)

file(GLOB HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

file(GLOB SOURCE
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.c")


set(CMAKE_SHARED_LINKER_FLAGS "-lz,--as-needed")
add_library(${PROJECT_NAME} ${HEADERS} ${SOURCE})
target_link_libraries(${PROJECT_NAME} Qt5::Core)
