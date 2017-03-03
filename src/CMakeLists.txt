set(CMAKE_INCLUDE_CURRENT_DIR ON)

# export version to source code
set(CKB_VERSION_STR ${ckb-next_VERSION})
configure_file(ckbnextconfig.h.in ckbnextconfig.h @ONLY)

# include this directory so that it's possible to write #include <ckbnextconfig.h> in source files
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# animations
if (WITH_GRADIENT)
    add_subdirectory(ckb-gradient)
endif ()
if (WITH_HEAT)
    add_subdirectory(ckb-heat)
endif ()
if (WITH_PINWHEEL)
    add_subdirectory(ckb-pinwheel)
endif ()
if (WITH_RAIN)
    add_subdirectory(ckb-rain)
endif ()
if (WITH_RANDOM)
    add_subdirectory(ckb-random)
endif ()
if (WITH_RIPPLE)
    add_subdirectory(ckb-ripple)
endif ()
if (WITH_WAVE)
    add_subdirectory(ckb-wave)
endif ()
if (LINUX AND WITH_MVIZ)
    add_subdirectory(ckb-mviz)
endif ()

# daemon and GUI
add_subdirectory(ckb-daemon)
# needs to be the last one to process
add_subdirectory(ckb)
