set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(StandardMathLibrary REQUIRED)

set(SOURCES
        main.c
        )

set(LIBS
        ${STANDARD_MATH_LIBRARY}
        )

add_executable(ckb-heat ${SOURCES})
target_link_libraries(ckb-heat ${LIBS})

install(TARGETS ckb-heat
        DESTINATION "${INSTALL_ANIMATIONS_DIR}"
        )
