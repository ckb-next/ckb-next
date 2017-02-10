set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(StandardMathLibrary REQUIRED)

set(SOURCES
        main.c
        )

set(LIBS
        ${STANDARD_MATH_LIBRARY}
        )

add_executable(ckb-random ${SOURCES})
target_link_libraries(ckb-random ${LIBS})

install(TARGETS ckb-random
        DESTINATION "${INSTALL_ANIMATIONS_DIR}"
        )
