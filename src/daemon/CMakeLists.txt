#
#        Copyright © 2017 Oleksii Vilchanskyi <alexvilchansky@yahoo.com>
#        
#        This file is part of ckb-next.
#
#        ckb-next is free software: you can redistribute it and/or modify
#        it under the terms of the GNU General Public License version 2
#        as published by the Free Software Foundation.
#
#        ckb-next is distributed in the hope that it will be useful,
#        but WITHOUT ANY WARRANTY; without even the implied warranty of
#        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#        GNU General Public License version 2 for more details.
#
#        You should have received a copy of the GNU General Public License
#        version 2 along with ckb-next. If not, see
#        <https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
#

# TODO: build ckb-anim as an interface library
# TODO: get rid of ifdefs in sources. This is CMake's task!
# TODO: use COMPONENT-wise installation instead of ifs()

# Declare the target
# Unfortunately, CMake doesn't allow doing this
# without specifying at least one source file, so here you have it
add_executable(ckb-next-daemon main.c)

# Find external dependencies
if (MACOS OR LINUX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    find_package(Iconv REQUIRED)
endif ()
if (MACOS)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    find_library(COREGRAPHICS_LIBRARY CoreGraphics)
    find_library(IOKIT_LIBRARY IOKit)
    if (COREFOUNDATION_LIBRARY-NOTFOUND OR COREGRAPHICS_LIBRARY-NOTFOUND OR IOKIT_LIBRARY-NOTFOUND)
        message(FATAL_ERROR "Some Apple libraries were not found:
CoreFoundation: ${COREFOUNDATION_LIBRARY}
CoreGraphics:   ${COREGRAPHICS_LIBRARY}
IOKit:          ${IOKIT_LIBRARY}")
    else ()
        message(STATUS "CoreFoundation found: ${COREFOUNDATION_LIBRARY}")
        message(STATUS "CoreGraphics found:   ${COREGRAPHICS_LIBRARY}")
        message(STATUS "IOKit found:          ${IOKIT_LIBRARY}")
    endif ()
        mark_as_advanced(COREFOUNDATION_LIBRARY COREGRAPHICS_LIBRARY IOKIT_LIBRARY)
elseif (LINUX)
    find_package(UDEV REQUIRED)
endif ()

# Declare target's source files
if (MACOS OR LINUX)
    target_sources(
        ckb-next-daemon
            PRIVATE
              device.c
              devnode.c
              input.c
              notify.c
              usb.c
              firmware.c
              profile.c
              keymap.c
              command.c
              device_vtable.c
              device_keyboard.c
              device_mouse.c
              led_keyboard.c
              led.c
              led_mouse.c
              profile_keyboard.c
              dpi.c
              profile_mouse.c
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
              structures.h
              dpi.h)
endif ()
if (MACOS)
    target_sources(
        ckb-next-daemon
            PRIVATE
              usb_mac.c
              input_mac.c
              input_mac_mouse.c
              extra_mac.c
              keymap_mac.h)
elseif (LINUX)
    target_sources(
        ckb-next-daemon
            PRIVATE
              usb_linux.c
              input_linux.c)
endif ()

# Declare target's include paths
if (MACOS OR LINUX)
    target_include_directories(
        ckb-next-daemon
            PRIVATE
              "${CMAKE_CURRENT_BINARY_DIR}"
              "${ICONV_INCLUDE_DIR}")
endif ()
if (MACOS)
    # Compiler knows better
elseif (LINUX)
    target_include_directories(
        ckb-next-daemon
            PRIVATE
              "${UDEV_INCLUDE_DIRS}")
endif ()

# Declare target's link dependencies
if (MACOS OR LINUX)
    target_link_libraries(
        ckb-next-daemon
            PRIVATE
              Threads::Threads
              "${ICONV_LIBRARIES}")
endif ()
if (MACOS)
    target_link_libraries(
        ckb-next-daemon
            PRIVATE
              "${COREFOUNDATION_LIBRARY}"
              "${COREGRAPHICS_LIBRARY}"
              "${IOKIT_LIBRARY}")
elseif (LINUX)
    target_link_libraries(
        ckb-next-daemon
            PRIVATE
              "${UDEV_LIBRARIES}")
endif ()

# Target compile features and options
# TODO: replace. cmake 5.5 doesn't know about it
#target_compile_features(
#    ckb-next-daemon
#        PRIVATE
#          c_std_11)

if (C_COMPILER_SUPPORTS_-Og)
    set(opt_lvl "-Og")
else ()
    set(opt_lvl "-O0")
endif ()

target_compile_options(
    ckb-next-daemon
        PRIVATE
          $<$<CONFIG:Debug>:${opt_lvl}>
          -fsigned-char
          -Wall
          -Wextra
          -Wfloat-equal
          -Wundef
          -Wshadow
          -Wpointer-arith
          -Wcast-align
          -Wstrict-prototypes
          -Wstrict-overflow=5
          -Wwrite-strings
          -Waggregate-return
          -Wcast-qual
          -Wswitch-default
          -Wswitch-enum
          -Wconversion
          -Wformat=2
          -Winit-self
          $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-save-temps>)

# Add sanitizers after all target information is known
add_sanitizers(ckb-next-daemon)

# We must be absolutely sure that daemons won't interfere with each other.
# Therefore we conduct a cleanup at install time before anything else.
# Package maintainers are supposed to disable SAFE_INSTALL and provide
# idiomatic solutions according to their package managers' guidelines to
# ensure similar behaviour.

if (SAFE_INSTALL)
    # We should close GUI first or daemon unloading might hang
    install(CODE [[message(STATUS "Looking for ckb or ckb-next GUI...")]])
    execute_process(
        COMMAND ps axco command          # imagine a pipe
        COMMAND grep "ckb\$\|ckb-next\$" # between these commands
        OUTPUT_VARIABLE gui_processes
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT (" ${gui_processes}" STREQUAL " "))
        string(REPLACE "\n" ";" gui_processes_list ${gui_processes})
        foreach (gui_process IN LISTS gui_processes_list)
            install(CODE [[message(STATUS "Terminating ${gui_process}...")]])
            install(CODE [[execute_process(COMMAND killall -9 ${gui_process})]])
        endforeach ()
    endif ()
endif ()

# No need to be repetitve, save init system.
set(init_system "unknown")

if (MACOS)
    set(init_system "launchd")

    # Note that sudo is required at configure time.
    # Also note it is not used at install time because
    # the install command is launched with sudo a priori.

    # TODO: test in general, and test whether sudo is required to list the job
    message(STATUS "Privileged access is required to list launchd jobs owned by root:")
    execute_process(
        COMMAND sudo launchctl list
        OUTPUT_VARIABLE launchctl_jobs)
    foreach (plist_name IN ITEMS "com.ckb.daemon.plist" "org.next.ckb.daemon.plist")
        string(FIND "${launchctl_jobs}" "${plist_name}" daemon_found)
        if (NOT (daemon_found EQUAL -1))
            install(CODE [[message(STATUS "Permanently unloading ${plist_name}...")]])
            install(CODE [[execute_process(COMMAND launchctl unload -w /Library/LaunchDaemons/${plist_name})]])
        endif ()
    endforeach ()
elseif (LINUX)
    # Determine init system first.

    # A way to check for systemd
    if (EXISTS "/run/systemd/system")
        message(STATUS "systemd detected")
        install(CODE [[message(STATUS "systemd detected")]])
        set(init_system "systemd")
        if (SAFE_INSTALL)
            foreach(daemon IN ITEMS "ckb-daemon" "ckb-next-daemon")
                execute_process(
                    COMMAND systemctl is-active ${daemon}.service
                    RESULT_VARIABLE daemon_active)
                if(daemon_active EQUAL 0)
                    install(CODE [[message(STATUS "Stoppping ${daemon}...")]])
                    install(CODE [[execute_process(COMMAND systemctl stop ${daemon}.service)]])
               endif()
            endforeach(daemon)
        endif ()
    endif ()

    # A way to check for OpenRC
    if (EXISTS "/run/openrc/softlevel")
        message(STATUS "OpenRC detected")
        install(CODE [[message(STATUS "OpenRC detected")]])
        set(init_system "OpenRC")
        if (SAFE_INSTALL)
            execute_process(
                COMMAND rc-status -a
                COMMAND grep 'started'
                COMMAND grep -o 'ckb\|ckb-next'
                RESULT_VARIABLE daemons
                OUTPUT_STRIP_TRAILING_WHITESPACE)
            if (NOT (" ${daemons}" STREQUAL " "))
                string(REPLACE "\n" ";" daemons_list ${daemons})
                foreach(daemon IN LISTS daemons_list)
                    install(CODE [[message(STATUS "Stoppping ${daemon}...")]])
                    install(CODE [[execute_process(COMMAND rc-service ${daemon} stop)]])
                endforeach(daemon)
           endif()
        endif ()
    endif ()

    # A way to check for Upstart
    execute_process(
        COMMAND initctl --version
        OUTPUT_VARIABLE initctl_output
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (${initctl_output} MATCHES "upstart")
        message(STATUS "Upstart detected")
        install(CODE [[message(STATUS "Upstart detected")]])
        set(init_system "Upstart")
        if (SAFE_INSTALL)
            foreach(daemon IN ITEMS "ckb-daemon" "ckb-next-daemon")
                execute_process(
                    COMMAND status ${daemon}
                    COMMAND grep '^${daemon} start'
                    RESULT_VARIABLE daemon_active)
                if(daemon_active EQUAL 0)
                    install(CODE [[message(STATUS "Stoppping ${daemon}...")]])
                    install(CODE [[execute_process(COMMAND service ${daemon} stop)]])
               endif()
            endforeach(daemon)
        endif()
    endif ()
endif ()

# Declare target's installation paths
if (MACOS)
    install(
        TARGETS ckb-next-daemon
        DESTINATION "/Library/Application Support") # TODO: check permissions
elseif (LINUX)
    install(
        TARGETS ckb-next-daemon
        DESTINATION "bin")
endif ()

# Declare target's auxiliary files installation paths
if (${init_system} STREQUAL "launchd")
    configure_file(
        "${CMAKE_SOURCE_DIR}/macos/org.next.ckb.daemon.plist"
        "${CMAKE_BINARY_DIR}/org.next.ckb.daemon.plist") # TODO: check plist's internals
    install(
        FILES "${CMAKE_BINARY_DIR}/org.next.ckb.daemon.plist"
        DESTINATION "/Library/LaunchDaemons"
        PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ)
elseif (${init_system} STREQUAL "systemd")
    install(
        FILES "${CMAKE_SOURCE_DIR}/linux/systemd/ckb-next-daemon.service"
        DESTINATION "/usr/lib/systemd/system"
        PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ)
elseif (${init_system} STREQUAL "Upstart")
    install(
        FILES "${CMAKE_SOURCE_DIR}/linux/upstart/ckb-next-daemon.conf"
        DESTINATION "/etc/init"
        PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ             GROUP_EXECUTE
        WORLD_READ             WORLD_EXECUTE)
elseif (${init_system} STREQUAL "OpenRC")
    install(
        FILES "${CMAKE_SOURCE_DIR}/linux/openrc/ckb-next-daemon"
        DESTINATION "/etc/init.d"
        PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ             GROUP_EXECUTE
        WORLD_READ             WORLD_EXECUTE)
elseif (${init_system} STREQUAL "unknown")
    message(FATAL_ERROR "No supported system service detected. \
Supported services are: systemd, launchd, OpenRC, Upstart.")
endif ()

