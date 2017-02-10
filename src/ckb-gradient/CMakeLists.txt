set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(StandardMathLibrary REQUIRED)

set(SOURCES
    main.c
    )

set(LIBS
    ${STANDARD_MATH_LIBRARY}
    )

add_executable(ckb-gradient ${SOURCES})
target_link_libraries(ckb-gradient ${LIBS})

install(TARGETS ckb-gradient
        DESTINATION "${INSTALL_ANIMATIONS_DIR}"
        )
