set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(StandardMathLibrary REQUIRED)

set(SOURCES
        main.c
        )

set(LIBS
        ${STANDARD_MATH_LIBRARY}
        )

add_executable(ckb-rain ${SOURCES})
target_link_libraries(ckb-rain ${LIBS})

install(TARGETS ckb-rain
        DESTINATION "${INSTALL_ANIMATIONS_DIR}"
        )
