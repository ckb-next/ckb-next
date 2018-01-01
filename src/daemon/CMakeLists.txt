#   Copyright (c) 2017 Oleksii Vilchanskyi <alexvilchansky@yahoo.com>
#   All rights reserved.
#   
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions are met:
#   
#   1. Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
#   2. Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#   3. Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission. 
#   
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#   POSSIBILITY OF SUCH DAMAGE.

# TODO: get rid of ifdefs in sources. This is CMake's task!
# TODO: use COMPONENT-wise installation instead of ifs()

add_executable(ckb-next-daemon "")

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
              main.c
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
              "${CMAKE_CURRENT_SOURCE_DIR}"
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
set_target_properties(
    ckb-next-daemon
        PROPERTIES
          C_STANDARD 11)

target_compile_options(
    ckb-next-daemon
      PRIVATE
        "${CKB_NEXT_COMMON_COMPILE_FLAGS}")

# Add sanitizers after all target information is known
add_sanitizers(ckb-next-daemon)

# We must be absolutely sure that daemons won't interfere with each other.
# Therefore we conduct a cleanup at install time before anything else.
# Package maintainers are supposed to disable SAFE_INSTALL and provide
# idiomatic solutions according to their package managers' guidelines to
# ensure similar behaviour.

if (SAFE_INSTALL)
    # We should close GUI first or daemon unloading might hang
    message(STATUS "Searching for running ckb and/or ckb-next GUI")
    execute_process(
        COMMAND ps axco command
        COMMAND egrep "^ckb$|^ckb-next$"
        OUTPUT_VARIABLE gui_processes
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (gui_processes)
        string(REPLACE "\n" ";" gui_processes_list "${gui_processes}")
        foreach (gui_process IN LISTS gui_processes_list)
            message(STATUS "Running ${gui_process} GUI detected")
            install(CODE "message(STATUS \"Terminating ${gui_process}\")")
            install(CODE "execute_process(COMMAND killall -9 ${gui_process})")
        endforeach ()
    endif ()
endif ()

if (SAFE_INSTALL)
    message(STATUS "NOTE:
    Privileged access is required for operations upon the daemon at
    configure time as it is owned by root. If this is unacceptable,
    disable SAFE_INSTALL and prepare the filesystem manually.")
endif ()

# No need to be repetitve, save init system
set(CKB_NEXT_INIT_SYSTEM "unknown" CACHE INTERNAL "")

if (MACOS)
    message(STATUS "launchd detected")
    set(CKB_NEXT_INIT_SYSTEM "launchd" CACHE INTERNAL "")

    # Import plist
    message(STATUS "Importing org.next.ckb.daemon.plist (${CKB_NEXT_INIT_SYSTEM})")
    configure_file(
        "${ckb-next_SOURCE_DIR}/macos/org.next.ckb.daemon.plist"
        "${CMAKE_CURRENT_BINARY_DIR}/service/org.next.ckb.daemon.plist" COPYONLY)

    if (SAFE_INSTALL)
        execute_process(
            COMMAND sudo launchctl list
            OUTPUT_VARIABLE launchctl_jobs)
        foreach (plist_name IN ITEMS "com.ckb.daemon" "org.next.ckb.daemon")
            string(FIND "${launchctl_jobs}" "${plist_name}" daemon_found)
            if (NOT ("${daemon_found}" EQUAL -1))
                message(STATUS "Running job with ${plist_name} plist detected")
                install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: permanently unloading ${plist_name}\")")
                install(CODE "execute_process(COMMAND sudo launchctl unload -w /Library/LaunchDaemons/${plist_name}.plist)")
            endif ()
        endforeach ()
    endif ()

elseif (LINUX)
    # Determine init system first

    # NOTE: upstart is checked first to correctly behave on systems which still have upstart
    # but it is not enabled by default and systemd is used instead. (Ubuntu 15.04+)

    # A way to check for upstart
    execute_process(
        COMMAND initctl --version
        OUTPUT_VARIABLE initctl_output
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    if ("${initctl_output}" MATCHES "upstart")
        message(STATUS "upstart detected")
        set(CKB_NEXT_INIT_SYSTEM "upstart" CACHE INTERNAL "")

        # Generate and import service
        message(STATUS "Generating and importing ckb-next-daemon.conf (${CKB_NEXT_INIT_SYSTEM})")
        configure_file(
            "${ckb-next_SOURCE_DIR}/linux/upstart/ckb-next-daemon.conf.in"
            "${CMAKE_CURRENT_BINARY_DIR}/service/ckb-next-daemon.conf")

        if (SAFE_INSTALL)
            foreach(daemon IN ITEMS "ckb-daemon" "ckb-next-daemon")
                execute_process(
                    COMMAND sudo service ${daemon} status
                    COMMAND egrep "^${daemon} start"
                    RESULT_VARIABLE daemon_active)
                if("${daemon_active}" EQUAL 0)
                    message(STATUS "Running ${daemon} detected")
                    install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: stopping ${daemon}\")")
                    install(CODE "execute_process(COMMAND sudo service ${daemon} stop)")
                endif()
                execute_process(
                    COMMAND sudo initctl show-config ${daemon}
                    COMMAND egrep "^  start on"
                    RESULT_VARIABLE daemon_enabled)
                if("${daemon_enabled}" EQUAL 0)
                    message(STATUS "Enabled ${daemon} detected")
                    install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: permanently disabling ${daemon}\")")
                    install(CODE "execute_process(COMMAND echo manual COMMAND sudo tee /etc/init/${daemon}.override)")
                endif()
            endforeach()
        endif()

    endif ()

    # A way to check for systemd
    if (EXISTS "/run/systemd/system")
        message(STATUS "systemd detected")
        set(CKB_NEXT_INIT_SYSTEM "systemd" CACHE INTERNAL "")

        # Generate and import service
        message(STATUS "Generating and importing ckb-next-daemon.service (${CKB_NEXT_INIT_SYSTEM})")
        configure_file(
            "${ckb-next_SOURCE_DIR}/linux/systemd/ckb-next-daemon.service.in"
            "${CMAKE_CURRENT_BINARY_DIR}/service/ckb-next-daemon.service")

        if (SAFE_INSTALL)
            foreach(daemon IN ITEMS "ckb-daemon" "ckb-next-daemon")
                execute_process(
                    COMMAND sudo systemctl is-active ${daemon}.service
                    RESULT_VARIABLE daemon_active)
                if("${daemon_active}" EQUAL 0)
                    message(STATUS "Running ${daemon} detected")
                    install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: stoppping ${daemon}\")")
                    install(CODE "execute_process(COMMAND sudo systemctl stop ${daemon}.service)")
                endif()
                execute_process(
                    COMMAND sudo systemctl is-enabled ${daemon}.service
                    RESULT_VARIABLE daemon_enabled)
                if("${daemon_enabled}" EQUAL 0)
                    message(STATUS "Enabled ${daemon} detected")
                    install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: permanently disabling ${daemon}\")")
                    install(CODE "execute_process(COMMAND sudo systemctl disable ${daemon}.service)")
                endif()
            endforeach()
        endif ()

    endif ()

    # A way to check for OpenRC
    if (EXISTS "/run/openrc/softlevel")
        message(STATUS "OpenRC detected")
        set(CKB_NEXT_INIT_SYSTEM "OpenRC" CACHE INTERNAL "")

        # Generate and import service
        message(STATUS "Generating and importing ckb-next-daemon (${CKB_NEXT_INIT_SYSTEM})")
        configure_file(
            "${ckb-next_SOURCE_DIR}/linux/openrc/ckb-next-daemon.in"
            "${CMAKE_CURRENT_BINARY_DIR}/service/ckb-next-daemon" @ONLY)

        if (SAFE_INSTALL)
            execute_process(
                COMMAND rc-status -a
                COMMAND egrep "started"
                COMMAND egrep -o "ckb-daemon|ckb-next-daemon"
                OUTPUT_VARIABLE daemons
                OUTPUT_STRIP_TRAILING_WHITESPACE)
            if (daemons)
                string(REPLACE "\n" ";" daemons_list "${daemons}")
                foreach(daemon IN LISTS daemons_list)
                    message(STATUS "Running ${daemon} detected")
                    install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: stoppping ${daemon}\")")
                    install(CODE "execute_process(COMMAND sudo rc-service ${daemon} stop)")
                endforeach()
            endif()
            execute_process(
                COMMAND rc-status -a
                COMMAND egrep -o "ckb-daemon|ckb-next-daemon"
                OUTPUT_VARIABLE daemons
                OUTPUT_STRIP_TRAILING_WHITESPACE)
            if (daemons)
                string(REPLACE "\n" ";" daemons_list "${daemons}")
                foreach(daemon IN LISTS daemons_list)
                    message(STATUS "Enabled ${daemon} detected")
                    install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: permanently disabling ${daemon}\")")
                    install(CODE "execute_process(COMMAND sudo rc-update del ${daemon} default)")
                endforeach()
            endif()
        endif ()

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
if ("${CKB_NEXT_INIT_SYSTEM}" STREQUAL "launchd")
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/service/org.next.ckb.daemon.plist"
        DESTINATION "/Library/LaunchDaemons"
        PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ)
elseif ("${CKB_NEXT_INIT_SYSTEM}" STREQUAL "systemd")
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/service/ckb-next-daemon.service"
        DESTINATION "/usr/lib/systemd/system"
        PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ)
elseif ("${CKB_NEXT_INIT_SYSTEM}" STREQUAL "upstart")
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/service/ckb-next-daemon.conf"
        DESTINATION "/etc/init"
        PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ             GROUP_EXECUTE
        WORLD_READ             WORLD_EXECUTE)
elseif ("${CKB_NEXT_INIT_SYSTEM}" STREQUAL "OpenRC")
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/service/ckb-next-daemon"
        DESTINATION "/etc/init.d"
        PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ             GROUP_EXECUTE
        WORLD_READ             WORLD_EXECUTE)
elseif ("${CKB_NEXT_INIT_SYSTEM}" STREQUAL "unknown")
    message(FATAL_ERROR "No supported system service detected.
    Supported services are: systemd, launchd, OpenRC, upstart.")
endif ()

if (LINUX)
  install(
    FILES "${CMAKE_SOURCE_DIR}/linux/udev/99-ckb-daemon.rules"
    DESTINATION "/etc/udev/rules.d/99-ckb-daemon.rules"
    PERMISSIONS
    OWNER_READ OWNER_WRITE
    GROUP_READ
    WORLD_READ)
endif ()

# Now, we should start and enable new services
if (SAFE_INSTALL)
    if ("${CKB_NEXT_INIT_SYSTEM}" STREQUAL "launchd")
        # Just installing the plist should be enough to enable the job at boot
        install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: loading org.next.ckb.daemon\")")
        install(CODE "execute_process(COMMAND sudo launchctl load -w /Library/LaunchDaemons/org.next.ckb.daemon.plist)")
    elseif ("${CKB_NEXT_INIT_SYSTEM}" STREQUAL "upstart")
        # Just installing the service should be enough to enable it at boot
        install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: refreshing services\")")
        install(CODE "execute_process(COMMAND sudo initctl reload-configuration)")
        install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: starting ckb-next-daemon\")")
        install(CODE "execute_process(COMMAND sudo service ckb-next-daemon start)")
    elseif ("${CKB_NEXT_INIT_SYSTEM}" STREQUAL "systemd")
        install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: refreshing services\")")
        install(CODE "execute_process(COMMAND sudo systemctl daemon-reload)")
        install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: enabling ckb-next-daemon\")")
        install(CODE "execute_process(COMMAND sudo systemctl enable ckb-next-daemon.service)")
        install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: starting ckb-next-daemon\")")
        install(CODE "execute_process(COMMAND sudo systemctl start ckb-next-daemon.service)")
    elseif ("${CKB_NEXT_INIT_SYSTEM}" STREQUAL "OpenRC")
        # No need to refresh
        install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: enabling ckb-next-daemon\")")
        install(CODE "execute_process(COMMAND sudo rc-update add ckb-next-daemon default)")
        install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: starting ckb-next-daemon\")")
        install(CODE "execute_process(COMMAND sudo rc-service ckb-next-daemon start)")
    endif ()
endif ()