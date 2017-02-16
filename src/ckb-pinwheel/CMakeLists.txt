set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(StandardMathLibrary REQUIRED)

set(SOURCES
        main.c
        )

set(LIBS
        ${STANDARD_MATH_LIBRARY}
        )

add_executable(ckb-pinwheel ${SOURCES})
target_link_libraries(ckb-pinwheel ${LIBS})

install(TARGETS ckb-pinwheel
        DESTINATION "${INSTALL_ANIMATIONS_DIR}")
