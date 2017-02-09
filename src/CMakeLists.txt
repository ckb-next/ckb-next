set(CMAKE_INCLUDE_CURRENT_DIR ON)

# export version to source code
set(CKB_VERSION_STR ${ckb-next_VERSION})

configure_file(ckbnextconfig.h.in ckbnextconfig.h @ONLY)

# include this directory so that it's possible to write #include <ckbnextconfig.h> in source files
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(ckb-daemon)
add_subdirectory(ckb)
# animation programs
#add_subdirectory(ckb-gradient)
#add_subdirectory(src/ckb-heat)
#add_subdirectory(src/ckb-mviz)
#add_subdirectory(src/ckb-pinwheel)
#add_subdirectory(src/ckb-rain)
#add_subdirectory(src/ckb-random)
#add_subdirectory(src/ckb-ripple)
#add_subdirectory(src/ckb-wave)
