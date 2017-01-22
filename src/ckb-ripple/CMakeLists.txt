project(ckb-ripple)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c99")
add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} m)

