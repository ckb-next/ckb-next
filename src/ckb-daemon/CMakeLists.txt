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

if (NOT LINUX_CUSTOM_INSTALL)
    install(TARGETS ckb-next-daemon
            RUNTIME DESTINATION "${INSTALL_CMAKE_DIR}"
            )
endif ()

if (APPLE)
    configure_file("${CMAKE_CURRENT_LIST_DIR}/ckb-next-daemon.plist.in"
            "${CMAKE_CURRENT_LIST_DIR}/ckb-next-daemon.plist")

    install(CODE [[execute_process(COMMAND /Applications/ckb-next.app/Contents/MacOS/ckb-next --close)]])
    install(CODE [[execute_process(COMMAND sudo launchctl unload /Library/LaunchDaemons/ckb-next-daemon.plist)]])
    install(FILES "${CMAKE_CURRENT_LIST_DIR}/ckb-next-daemon.plist"
            DESTINATION "/Library/LaunchDaemons"
            PERMISSIONS OWNER_READ OWNER_WRITE
            GROUP_READ
            WORLD_READ
            )
    install(CODE [[execute_process(COMMAND sudo launchctl load /Library/LaunchDaemons/ckb-next-daemon.plist)]])
elseif (LINUX AND NOT LINUX_CUSTOM_INSTALL)
    if (NOT "${CMAKE_INSTALL_PREFIX}/${INSTALL_RUNTIME_DIR}" STREQUAL "/usr/bin")
        # symlink to /usr/bin
        makeLink("${INSTALL_CMAKE_DIR}/ckb-next-daemon" "/usr/bin/ckb-next-daemon" "ckb-next-daemon")
    endif ()

    execute_process(COMMAND systectl --version RESULT_VARIABLE SYSTEMD_FOUND)
    execute_process(COMMAND initctl version RESULT_VARIABLE UPSTART_FOUND)
    execute_process(COMMAND openrc --version RESULT_VARIABLE OPENRC_FOUND)

    install(CODE [[execute_process(COMMAND /usr/bin/ckb-next --close)]])

    if (SYSTEMD_FOUND STREQUAL "0")
        install(CODE [[message(STATUS "System service: systemd detected")]])
        install(CODE [[execute_process(COMMAND sudo systemctl stop ckb-next-daemon)]])
        install(FILES "${CMAKE_SOURCE_DIR}/linux/systemd/ckb-next-daemon.service"
                DESTINATION "/usr/lib/systemd/system"
                PERMISSIONS OWNER_READ OWNER_WRITE
                GROUP_READ
                WORLD_READ
                )
        install(CODE [[execute_process(COMMAND sudo systemctl daemon-reload)]])
        install(CODE [[execute_process(COMMAND sudo systemctl enable ckb-next-daemon)]])
        install(CODE [[execute_process(COMMAND sudo systemctl start ckb-next-daemon)]])
    elseif (UPSTART_FOUND STREQUAL "0")
        install(CODE [[message(STATUS "System service: Upstart detected")]])
        install(CODE [[execute_process(COMMAND sudo service ckb-next-daemon stop)]])
        install(FILES "${CMAKE_SOURCE_DIR}/linux/upstart/ckb-next-daemon.conf"
                DESTINATION "/etc/init"
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
                )
        install(CODE [[execute_process(COMMAND sudo service ckb-next-daemon start)]])
    elseif (OPENRC_FOUND STREQUAL "0")
        install(CODE [[message(STATUS "System service: OpenRC detected")]])
        install(CODE [[execute_process(COMMAND sudo rc-service stop ckb-next-daemon)]])
        install(FILES "${CMAKE_SOURCE_DIR}/linux/openrc/ckb-next-daemon"
                DESTINATION "/etc/init.d"
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
                )
        install(CODE [[execute_process(COMMAND sudo rc-service start ckb-next-daemon)]])
        install(CODE [[execute_process(COMMAND sudo rc-service add ckb-next-daemon)]])
    else ()
        message(FATAL_ERROR "No supported system service detected.\
Supported services are: systemd, launchd, OpenRC, Upstart.\
Open an issue on Github to support your configuration.")
    endif ()
endif ()
