set(CMAKE_INCLUDE_CURRENT_DIR ON)

# ------------------------------------------------------------------------------
# find libraries
if (APPLE)
    # find Iconv
    find_package(ICONV REQUIRED)
    include_directories(SYSTEM ${ICONV_INCLUDE_DIRS})

    # find Apple frameworks
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    find_library(COREGRAPHICS_LIBRARY CoreGraphics)
    find_library(IOKIT_LIBRARY IOKit)
    mark_as_advanced(
            COREFOUNDATION_LIBRARY
            COREGRAPHICS_LIBRARY
            IOKIT_LIBRARY
    )
elseif (LINUX)
    # find udev
    find_package(UDEV REQUIRED)
    include_directories(SYSTEM ${UDEV_INCLUDE_DIRS})
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

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# ------------------------------------------------------------------------------
# mark libraries
set(LIBS
        Threads::Threads
        )

if (APPLE)
    list(APPEND LIBS
            ${COREFOUNDATION_LIBRARY}
            ${COREGRAPHICS_LIBRARY}
            ${IOKIT_LIBRARY}
            ${ICONV_LIBRARIES}
            )
elseif (LINUX)
    list(APPEND LIBS
            ${UDEV_LIBRARIES}
            )
endif ()

# ------------------------------------------------------------------------------
# set up targets
add_executable(ckb-next-daemon ${SOURCES} ${HEADERS})
target_link_libraries(ckb-next-daemon ${LIBS})

install(TARGETS ckb-next-daemon
        RUNTIME DESTINATION "${INSTALL_CMAKE_DIR}")

if (APPLE)
    configure_file("${CMAKE_CURRENT_LIST_DIR}/ckb-next-daemon.plist.in"
            "${CMAKE_BINARY_DIR}/ckb-next-daemon.plist" @ONLY)

    install(CODE [[execute_process(COMMAND sudo launchctl unload /Library/LaunchDaemons/ckb-next-daemon.plist)]])
    install(FILES "${CMAKE_BINARY_DIR}/ckb-next-daemon.plist"
            DESTINATION "/Library/LaunchDaemons"
            PERMISSIONS OWNER_READ OWNER_WRITE
            GROUP_READ
            WORLD_READ)
elseif (LINUX AND NOT LINUX_CUSTOM_INSTALL)
        # ...
endif ()
