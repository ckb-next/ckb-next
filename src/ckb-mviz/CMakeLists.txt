set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(PulseAudio REQUIRED)
find_package(StandardMathLibrary REQUIRED)

include_directories(${PULSEAUDIOSIMPLE_INCLUDE_DIRS})

set(HEADERS
        _kiss_fft_guts.h
        kiss_fft.h
        kiss_fftr.h
        )

set(SOURCES
        main.c
        kiss_fft.c
        kiss_fftr.c
        )

set(LIBS
        ${STANDARD_MATH_LIBRARY}
        ${PULSEAUDIOSIMPLE_LIBRARIES}
        )

add_executable(ckb-mviz ${SOURCES} ${HEADERS})
target_link_libraries(ckb-mviz ${LIBS})

install(TARGETS ckb-mviz
        DESTINATION "${INSTALL_ANIMATIONS_DIR}"
        )
