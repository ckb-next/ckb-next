#   Copyright 2017-2018 ckb-next Development Team <ckb-next@googlegroups.com>
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
add_custom_target(daemon DEPENDS ckb-next-daemon)

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
              command.c
              device.c
              device_keyboard.c
              device_mouse.c
              device_bragi.c
              device_vtable.c
              devnode.c
              dpi.c
              dpi_legacy.c
              dpi_bragi.c
              firmware.c
              input.c
              keymap.c
              keymap_patch.c
              led.c
              led_keyboard.c
              led_mouse.c
              led_mousepad.c
              led_wireless.c
              led_bragi.c
              main.c
              notify.c
              profile.c
              profile_keyboard.c
              profile_mouse.c
              usb.c
              usb_nxp.c
              usb_legacy.c
              usb_bragi.c
              bragi_common.c
              bragi_notification.c
              command.h
              device.h
              devnode.h
              dpi.h
              dpi_legacy.h
              dpi_bragi.h
              firmware.h
              includes.h
              input.h
              input_mac_vhid.h
              keymap.h
              keymap_patch.h
              led.h
              legacykb_proto.h
              notify.h
              nxp_proto.h
              os.h
              profile.h
              request_hid_mac.h
              structures.h
              usb.h
              usb_nxp.h
              usb_legacy.h
              bragi_proto.h
              usb_bragi.h
              bragi_common.h
              bragi_notification.h
    )
endif ()
if (MACOS)
    target_sources(
        ckb-next-daemon
            PRIVATE
              usb_mac.c
              input_mac.c
              input_mac_legacy.c
              input_mac_mouse_legacy.c
              extra_mac.c
              request_hid_mac.c
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
        "${CKB_NEXT_COMMON_COMPILE_FLAGS}"
        "${CKB_NEXT_EXTRA_C_FLAGS}")

if (MAC_LEGACY)
    message(WARNING "Building a legacy daemon")
    target_compile_definitions(
        ckb-next-daemon
            PRIVATE
                OS_MAC_LEGACY)
endif ()

# Add sanitizers after all target information is known
add_sanitizers(ckb-next-daemon)

# We must be absolutely sure that daemons won't interfere with each other.
# Therefore we conduct a cleanup at install time before anything else.
# Distro package maintainers are not supposed to enable SAFE_INSTALL and
# provide idiomatic solutions according to their package managers' guidelines
# to ensure similar behaviour.

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

    # Import plists
    message(STATUS "Importing org.ckb-next.daemon.plist (${CKB_NEXT_INIT_SYSTEM})")
    configure_file(
        "${ckb-next_SOURCE_DIR}/macos/org.ckb-next.daemon.plist"
        "${CMAKE_CURRENT_BINARY_DIR}/service/org.ckb-next.daemon.plist"
        COPYONLY)

    configure_file(
        "${ckb-next_SOURCE_DIR}/macos/org.ckb-next.daemon_agent.plist"
        "${CMAKE_CURRENT_BINARY_DIR}/service/org.ckb-next.daemon_agent.plist"
        COPYONLY)

    if (SAFE_INSTALL)
        execute_process(
            COMMAND sudo launchctl list
            OUTPUT_VARIABLE launchctl_jobs)
        foreach (plist_name IN ITEMS "com.ckb.daemon" "org.next.ckb.daemon" "org.ckb-next.daemon")
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

    # Used to prevent false SysVinit detection on systems that have /etc/inittab with systemd
    set(DISALLOW_SYSVINIT FALSE)

    # NOTE: upstart is checked first to correctly behave on systems which still have upstart
    # but it is not enabled by default and systemd is used instead. (Ubuntu 15.04+)

    # A way to check for upstart
    execute_process(
        COMMAND initctl --version
        OUTPUT_VARIABLE initctl_output
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    if ("${initctl_output}" MATCHES "upstart" OR "upstart" IN_LIST FORCE_INIT_SYSTEM)
        message(STATUS "upstart detected")
        set(CKB_NEXT_INIT_SYSTEM "upstart" CACHE INTERNAL "")
        set(DISALLOW_SYSVINIT TRUE)

        # Generate and import service
        message(STATUS "Generating and importing ckb-next-daemon.conf (${CKB_NEXT_INIT_SYSTEM})")
        configure_file(
            "${ckb-next_SOURCE_DIR}/linux/upstart/ckb-next-daemon.conf.in"
            "${CMAKE_CURRENT_BINARY_DIR}/service/ckb-next-daemon.conf"
            @ONLY)

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
    if (EXISTS "/run/systemd/system" OR "systemd" IN_LIST FORCE_INIT_SYSTEM)
        message(STATUS "systemd detected")
        set(CKB_NEXT_INIT_SYSTEM "systemd" CACHE INTERNAL "")
        set(SYSTEMD_UNIT_INSTALL_DIR "/usr/lib/systemd/system" CACHE STRING "Where to install systemd unit files.")
        set(DISALLOW_SYSVINIT TRUE)

        # Generate and import service
        message(STATUS "Generating and importing ckb-next-daemon.service (${CKB_NEXT_INIT_SYSTEM})")
        configure_file(
            "${ckb-next_SOURCE_DIR}/linux/systemd/ckb-next-daemon.service.in"
            "${CMAKE_CURRENT_BINARY_DIR}/service/ckb-next-daemon.service"
            @ONLY)

        if (SAFE_INSTALL)
            foreach (daemon IN ITEMS "ckb-daemon" "ckb-next-daemon")
                execute_process(
                    COMMAND sudo systemctl is-active ${daemon}.service
                    RESULT_VARIABLE daemon_active)
                if ("${daemon_active}" EQUAL 0)
                    message(STATUS "Running ${daemon} detected")
                    install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: stopping ${daemon}\")")
                    install(CODE "execute_process(COMMAND sudo systemctl stop ${daemon}.service)")
                endif ()
                execute_process(
                    COMMAND sudo systemctl is-enabled ${daemon}.service
                    RESULT_VARIABLE daemon_enabled)
                if ("${daemon_enabled}" EQUAL 0)
                    message(STATUS "Enabled ${daemon} detected")
                    install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: permanently disabling ${daemon}\")")
                    install(CODE "execute_process(COMMAND sudo systemctl disable ${daemon}.service)")
                endif ()
            endforeach ()
        endif ()

    endif ()

    # A way to check for OpenRC
    if (EXISTS "/run/openrc/softlevel" OR "openrc" IN_LIST FORCE_INIT_SYSTEM)
        message(STATUS "OpenRC detected")
        set(CKB_NEXT_INIT_SYSTEM "OpenRC" CACHE INTERNAL "")
        set(DISALLOW_SYSVINIT TRUE)

        # Generate and import service
        message(STATUS "Generating and importing ckb-next-daemon (${CKB_NEXT_INIT_SYSTEM})")
        configure_file(
            "${ckb-next_SOURCE_DIR}/linux/openrc/ckb-next-daemon.in"
            "${CMAKE_CURRENT_BINARY_DIR}/service/ckb-next-daemon"
            @ONLY)

        if (SAFE_INSTALL)
            execute_process(
                COMMAND rc-status -a
                COMMAND egrep "started"
                COMMAND egrep -o "ckb-daemon|ckb-next-daemon"
                OUTPUT_VARIABLE daemons
                OUTPUT_STRIP_TRAILING_WHITESPACE)
            if (daemons)
                string(REPLACE "\n" ";" daemons_list "${daemons}")
                foreach (daemon IN LISTS daemons_list)
                    message(STATUS "Running ${daemon} detected")
                    install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: stopping ${daemon}\")")
                    install(CODE "execute_process(COMMAND sudo rc-service ${daemon} stop)")
                endforeach ()
            endif ()
            execute_process(
                COMMAND rc-status -a
                COMMAND egrep -o "ckb-daemon|ckb-next-daemon"
                OUTPUT_VARIABLE daemons
                OUTPUT_STRIP_TRAILING_WHITESPACE)
            if (daemons)
                string(REPLACE "\n" ";" daemons_list "${daemons}")
                foreach (daemon IN LISTS daemons_list)
                    message(STATUS "Enabled ${daemon} detected")
                    install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: permanently disabling ${daemon}\")")
                    install(CODE "execute_process(COMMAND sudo rc-update del ${daemon} default)")
                endforeach ()
            endif ()
        endif ()

    endif ()

    # A way to check for SysVinit
    # Fall back to this only if any of the above haven't been found
    if ((EXISTS "/etc/inittab" AND EXISTS "/lib/lsb/init-functions" AND NOT DISALLOW_SYSVINIT) OR "sysvinit" IN_LIST FORCE_INIT_SYSTEM)
        message(STATUS "SysVinit detected")
        set(CKB_NEXT_INIT_SYSTEM "SysVinit" CACHE INTERNAL "")

        # Generate and import service
        message(STATUS "Generating and importing ckb-next-daemon (${CKB_NEXT_INIT_SYSTEM})")
        configure_file(
            "${ckb-next_SOURCE_DIR}/linux/sysvinit/ckb-next-daemon.in"
            "${CMAKE_CURRENT_BINARY_DIR}/service/ckb-next-daemon.sh"
            @ONLY)

        if (SAFE_INSTALL)
            if (EXISTS "/dev/input/ckb0/pid")
                message(STATUS "Running ckb-next-daemon detected")
                install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: stopping ${daemon}\")")
                install(CODE "execute_process(COMMAND sudo /etc/init.d/ckb-next-daemon.sh stop)")
            endif ()
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
        FILES "${CMAKE_CURRENT_BINARY_DIR}/service/org.ckb-next.daemon.plist"
        DESTINATION "/Library/LaunchDaemons"
        PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ)
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/service/org.ckb-next.daemon_agent.plist"
        DESTINATION "/Library/LaunchAgents"
        PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ)
elseif ("${CKB_NEXT_INIT_SYSTEM}" STREQUAL "systemd")
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/service/ckb-next-daemon.service"
        DESTINATION "${SYSTEMD_UNIT_INSTALL_DIR}"
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
elseif ("${CKB_NEXT_INIT_SYSTEM}" STREQUAL "SysVinit")
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/service/ckb-next-daemon.sh"
        DESTINATION "/etc/init.d"
        PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ             GROUP_EXECUTE
        WORLD_READ             WORLD_EXECUTE)
elseif ("${CKB_NEXT_INIT_SYSTEM}" STREQUAL "unknown")
    message(WARNING "No supported system service detected.
    Supported services are: systemd, launchd, OpenRC, upstart, SysVinit.")
endif ()

if (LINUX)
  if (SAFE_INSTALL)
    install(CODE "
    if (EXISTS \"/etc/udev/rules.d/99-ckb-daemon.rules\")
        file(SHA256 \"/etc/udev/rules.d/99-ckb-daemon.rules\" SHASUM)
        if(\"\${SHASUM}\" STREQUAL \"956c99d14effd3292a9575b1a918a598129121babd1296bc8772fcf809b7038a\"
            OR \"\${SHASUM}\" STREQUAL \"b135705d4709ae39e6e2c736c2bc80b88255ed11c582e86fae5086e90f256913\")
            message(\"-- Removing old 99-ckb-daemon.rules\")
            file(REMOVE \"/etc/udev/rules.d/99-ckb-daemon.rules\")
        else()
            message(\"-- Not removing old 99-ckb-daemon.rules because it has been modified.\")
        endif()
    endif()
    if (EXISTS \"/usr/lib/udev/rules.d/99-ckb-next-daemon.rules\" AND NOT IS_SYMLINK \"/lib\")
        message(\"-- Removing /usr/lib/udev/rules.d/99-ckb-next-daemon.rules\")
        file(REMOVE \"/usr/lib/udev/rules.d/99-ckb-next-daemon.rules\")
    endif() ")
  endif ()
  install(
    FILES "${CMAKE_SOURCE_DIR}/linux/udev/99-ckb-next-daemon.rules"
    DESTINATION "${UDEV_RULE_DIRECTORY}"
    PERMISSIONS
    OWNER_READ OWNER_WRITE
    GROUP_READ
    WORLD_READ)
endif ()

if (MACOS AND NOT MAC_LEGACY)
  install(
    DIRECTORY "${ckb-next_SOURCE_DIR}/macos/org.pqrs.driver.Karabiner.VirtualHIDDevice.v060800.kext"
    DESTINATION "/Library/Extensions"
    PATTERN "*"
    PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE)
endif ()

# Now, we should start and enable new services
if (SAFE_INSTALL)
    if ("${CKB_NEXT_INIT_SYSTEM}" STREQUAL "launchd")
        # Just installing the plist should be enough to enable the job at boot
        install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: loading org.ckb-next.daemon\")")
        install(CODE "execute_process(COMMAND sudo launchctl load -w /Library/LaunchDaemons/org.ckb-next.daemon.plist)")
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
    elseif ("${CKB_NEXT_INIT_SYSTEM}" STREQUAL "SysVinit")
        # update-rc.d is a debian utility to handle sysvinit links
        find_program(UPDATE_RC_EXISTS "update-rc.d")
        if (UPDATE_RC_EXISTS)
            install(CODE "execute_process(COMMAND sudo update-rc.d ckb-next-daemon.sh defaults)")
        endif ()
        # Just running the script should be enough
        install(CODE "message(STATUS \"${CKB_NEXT_INIT_SYSTEM}: starting ckb-next-daemon\")")
        install(CODE "execute_process(COMMAND sudo /etc/init.d/ckb-next-daemon.sh start)")
    endif ()
endif ()
