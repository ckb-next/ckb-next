project(ckb-mviz)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c99")

set(HEADERS _kiss_fft_guts.h kiss_fft.c kiss_fftr.h)

set(SOURCES main.c kiss_fft.c kiss_fftr.c)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
target_link_libraries(${PROJECT_NAME} pulse-simple m)