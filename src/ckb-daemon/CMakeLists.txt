set(CMAKE_INCLUDE_CURRENT_DIR ON)

# ------------------------------------------------------------------------------
# find libraries
if (APPLE)
    # find Iconv
    find_package(ICONV REQUIRED)
    include_directories(${ICONV_INCLUDE_DIRS})

    # find Apple frameworks
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    find_library(COREGRAPHICS_LIBRARY CoreGraphics)
    find_library(IOKIT_LIBRARY IOKit)
    mark_as_advanced(
            COREFOUNDATION_LIBRARY
            COREGRAPHICS_LIBRARY
            IOKIT_LIBRARY
    )
endif ()

if (LINUX)
    # find udev
    find_package(UDEV REQUIRED)
    include_directories(${UDEV_INCLUDE_DIRS})
endif ()

# find POSIX Threads (for both systems)
find_package(Threads REQUIRED)

# ------------------------------------------------------------------------------
# mark all source files
set(SOURCES
        device.c
        devnode.c
        input_linux.c
        input_mac.c
        input.c
        main.c
        notify.c
        usb_linux.c
        usb_mac.c
        usb.c
        firmware.c
        profile.c
        extra_mac.c
        keymap.c
        command.c
        device_vtable.c
        device_keyboard.c
        device_mouse.c
        led_keyboard.c
        led.c
        led_mouse.c
        input_mac_mouse.c
        profile_keyboard.c
        dpi.c
        profile_mouse.c
        )

set(HEADERS
        device.h
        devnode.h
        includes.h
        input.h
        led.h
        notify.h
        os.h
        usb.h
        firmware.h
        profile.h
        command.h
        keymap.h
        keymap_mac.h
        structures.h
        dpi.h
        )

# ------------------------------------------------------------------------------
# mark libraries
set(LIBS
        Threads::Threads
        )

if (APPLE)
    set(APPLE_LIBS
            ${COREFOUNDATION_LIBRARY}
            ${COREGRAPHICS_LIBRARY}
            ${IOKIT_LIBRARY}
            ${ICONV_LIBRARIES}
            )
endif ()

if (LINUX)
    set(LINUX_LIBS
            ${UDEV_LIBRARIES}
            )
endif ()

# ------------------------------------------------------------------------------
# set up targets
if (APPLE)
    add_executable(ckb-next-daemon ${SOURCES} ${HEADERS})
    target_link_libraries(ckb-next-daemon ${LIBS} ${APPLE_LIBS})
endif ()

if (LINUX)
    add_executable(${PROJECT_NAME}-daemon ${SOURCES} ${HEADERS})
    target_link_libraries(${PROJECT_NAME}-daemon ${LIBS} ${LINUX_LIBS})
    set_target_properties(${PROJECT_NAME}-daemon PROPERTIES THREADS_PREFER_PTHREAD_FLAG ON)
endif ()
