# Export version etc. to source code
configure_file(ckbnextconfig.h.in ckbnextconfig.h)

# Include this directory so that it's possible to write #include <ckbnextconfig.h> in source files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

if (WITH_GUI)
    add_subdirectory(gui)
endif ()

add_subdirectory(daemon)

if (WITH_ANIMATIONS)
    add_subdirectory(animations)
endif ()