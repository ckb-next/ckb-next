set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(StandardMathLibrary REQUIRED)

set(SOURCES
        main.c
        )

set(LIBS
        ${STANDARD_MATH_LIBRARY}
        )

add_executable(ckb-ripple ${SOURCES})
target_link_libraries(ckb-ripple ${LIBS})

install(TARGETS ckb-ripple
        DESTINATION "${INSTALL_ANIMATIONS_DIR}")
