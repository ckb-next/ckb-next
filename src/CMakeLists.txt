set(CMAKE_INCLUDE_CURRENT_DIR ON)

# export version to source code
set(CKB_VERSION_STR ${ckb-next_VERSION})
add_definitions(-DCKB_VERSION_STR)

configure_file(ckbnextconfig.h.in ckbnextconfig.h @ONLY)

# include this directory so that it's possible to write #include <ckbnextconfig.h> in source files
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# animations
add_subdirectory(ckb-gradient)
add_subdirectory(ckb-heat)
add_subdirectory(ckb-pinwheel)
add_subdirectory(ckb-rain)
add_subdirectory(ckb-random)
add_subdirectory(ckb-ripple)
add_subdirectory(ckb-wave)

if (LINUX)
    add_subdirectory(ckb-mviz)
endif ()

# daemon and GUI
add_subdirectory(ckb-daemon)
# needs to be the last one to process
add_subdirectory(ckb)
