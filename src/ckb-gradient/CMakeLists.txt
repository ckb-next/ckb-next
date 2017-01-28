project(ckb-gradient)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/ckb-animations)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c99")
add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} m)

