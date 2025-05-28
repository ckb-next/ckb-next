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

add_executable(ckb-next "")
set_property(TARGET ckb-next PROPERTY AUTOGEN_TARGET_DEPENDS ckb-next-lrelease)
add_custom_target(gui DEPENDS ckb-next)

# Declare we want macOS Bundle
if (MACOS)
    set_target_properties(
        ckb-next
            PROPERTIES
              MACOSX_BUNDLE TRUE)
endif ()

# Prepare external dependencies' paths
if (USE_BREW_QT5)
    execute_process(
        COMMAND brew ls --versions qt5
        RESULT_VARIABLE qt5_found)
    if (NOT ("${qt5_found}" EQUAL 0))
        message(FATAL_ERROR "Homebrew's 'qt5' package is not installed.")
    endif ()
    execute_process(
        COMMAND brew --prefix qt5
        OUTPUT_VARIABLE qt5_path
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    list(APPEND CMAKE_PREFIX_PATH "${qt5_path}")
endif ()

if (USE_BREW_QUAZIP)
    execute_process(
        COMMAND brew ls --versions quazip
        RESULT_VARIABLE quazip_found)
    if (NOT ("${quazip_found}" EQUAL 0))
        message(FATAL_ERROR "Homebrew's 'quazip' package is not installed.")
    endif ()
    execute_process(
        COMMAND brew --prefix quazip
        OUTPUT_VARIABLE quazip_path
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    list(APPEND CMAKE_PREFIX_PATH "${quazip_path}")
endif ()

# Find external dependencies
if (MACOS OR LINUX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    find_package(QT NAMES ${QT_LIST} REQUIRED COMPONENTS Core Widgets Network Gui)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Network Gui)
    if(QT_VERSION VERSION_GREATER_EQUAL 6)
        find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)
    endif()
    find_package(ZLIB REQUIRED)
    find_package(QuaZip REQUIRED)
endif ()
if (MACOS)
    find_library(FOUNDATION_LIBRARY Foundation)
    find_library(COREAUDIO_LIBRARY CoreAudio)
    find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
    if (FOUNDATION_LIBRARY-NOTFOUND OR COREAUDIO_LIBRARY-NOTFOUND OR AUDIOTOOLBOX_LIBRARY-NOTFOUND)
        message(FATAL_ERROR "Some Apple libraries were not found:
Foundation:   ${FOUNDATION_LIBRARY}
CoreAudio:    ${COREAUDIO_LIBRARY}
AudioToolbox: ${AUDIOTOOLBOX_LIBRARY}")
    else ()
        message(STATUS "Foundation found:   ${FOUNDATION_LIBRARY}")
        message(STATUS "CoreAudio found:    ${COREAUDIO_LIBRARY}")
        message(STATUS "AudioToolbox found: ${AUDIOTOOLBOX_LIBRARY}")
    endif ()
        mark_as_advanced(FOUNDATION_LIBRARY COREAUDIO_LIBRARY AUDIOTOOLBOX_LIBRARY)
elseif (LINUX)
    if (QT_VERSION VERSION_LESS "5.12")
        message(FATAL "ckb-next requires Qt >= 5.12")
    endif ()
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS DBus)
    if (QT_VERSION VERSION_LESS "6")
        find_package(Qt5 REQUIRED COMPONENTS X11Extras)
    endif ()
    message(STATUS "Using Qt${QT_VERSION_MAJOR}")
endif ()

# Automatically handle MOC, UIC and RCC
 set_target_properties(
     ckb-next
         PROPERTIES
           AUTOMOC ON
           AUTOUIC ON
           AUTORCC ON)

# TODO: get rid of os.h and platform ifdefs
# Declare target's source files
if (MACOS OR LINUX)
    target_sources(
        ckb-next
            PRIVATE
              animadddialog.cpp
              animdetailsdialog.cpp
              animscript.cpp
              animsettingdialog.cpp
              autorun.cpp
              batterysystemtrayicon.cpp
              ckbmainbackgroundcolour.cpp
              ckbsettings.cpp
              ckbsettingswriter.cpp
              ckbsystemquirks.cpp
              ckbupdater.cpp
              ckbupdaterwidget.cpp
              ckbversionnumber.cpp
              clickeventpushbutton.cpp
              colorbutton.cpp
              colormap.cpp
              extrasettingswidget.cpp
              fwupgradedialog.cpp
              gradientbutton.cpp
              gradientdialog.cpp
              gradientdialogwidget.cpp
              idletimer.cpp
              kbanim.cpp
              kbanimwidget.cpp
              kbbind.cpp
              kbbindwidget.cpp
              kb.cpp
              kbfirmware.cpp
              kblight.cpp
              kblightwidget.cpp
              kbmanager.cpp
              kbmode.cpp
              kbmodeeventmgr.cpp
              kbwindowinfo.cpp
              kbwindowinfomodel.cpp
              kbperf.cpp
              kbprofile.cpp
              kbprofiledialog.cpp
              kbwidget.cpp
              keyaction.cpp
              keymap.cpp
              keywidget.cpp
              keywidgetdebugger.cpp
              kperfwidget.cpp
              legacylightwidget.cpp
              macroreader.cpp
              macrostringeditdialog.cpp
              macrotablemodel.cpp
              main.cpp
              mainwindow.cpp
              modeselectdialog.cpp
              modelisttablemodel.cpp
              modelistwidget.cpp
              mperfwidget.cpp
              nouserinputtextedit.cpp
              nowheelcombobox.cpp
              rebindwidget.cpp
              rlistwidget.cpp
              settingswidget.cpp
              animadddialog.h
              animdetailsdialog.h
              animscript.h
              animsettingdialog.h
              autorun.h
              ckbmainbackgroundcolour.h
              ckbsettings.h
              ckbsettingswriter.h
              ckbsystemquirks.h
              ckbsystemtrayicon.h
              ckbupdater.h
              ckbupdaterwidget.h
              ckbversionnumber.h
              clickeventpushbutton.h
              colorbutton.h
              colormap.h
              extrasettingswidget.h
              fwupgradedialog.h
              gradientbutton.h
              gradientdialog.h
              gradientdialogwidget.h
              idletimer.h
              kbanim.h
              kbanimwidget.h
              kbbind.h
              kbbindwidget.h
              kbfirmware.h
              kb.h
              kblight.h
              kblightwidget.h
              kbmanager.h
              kbmode.h
              kbmodeeventmgr.h
              kbperf.h
              kbprofiledialog.h
              kbprofile.h
              kbwidget.h
              keyaction.h
              keymap.h
              keywidget.h
              keywidgetdebugger.h
              keywidgetlayout.h
              kperfwidget.h
              legacylightwidget.h
              macroline.h
              macroreader.h
              macrostringeditdialog.cpp
              macrotablemodel.h
              mainwindow.h
              media.h
              modeselectdialog.h
              modelisttablemodel.h
              modelistwidget.h
              monotonicclock.h
              mperfwidget.h
              nouserinputtextedit.h
              nowheelcombobox.h
              compat/qoverloadlegacy.h
              compat/qrand.h
              rebindwidget.h
              rlistwidget.h
              settingswidget.h
              kbwindowinfo.h
              kbwindowinfomodel.h
              xcb/xwindowdetector.h
              xcb/xwindowinfo.h
              animadddialog.ui
              animdetailsdialog.ui
              animsettingdialog.ui
              ckbupdaterwidget.ui
              extrasettingswidget.ui
              fwupgradedialog.ui
              gradientdialog.ui
              kbanimwidget.ui
              kbbindwidget.ui
              kblightwidget.ui
              kbmodeeventmgr.ui
              kbprofiledialog.ui
              kbwidget.ui
              keywidgetdebugger.ui
              kperfwidget.ui
              legacylightwidget.ui
              mainwindow.ui
              macrostringeditdialog.ui
              modeselectdialog.ui
              mperfwidget.ui
              rebindwidget.ui
              settingswidget.ui)
endif ()
if (MACOS)
    target_sources(
        ckb-next
            PRIVATE
              media_mac.m)
elseif (LINUX)
    find_package(PulseAudio)
    if (PULSEAUDIO_FOUND)
        target_compile_definitions(
            ckb-next
                PRIVATE
                    USE_LIBPULSE)
    endif ()

    if (USE_XCB)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(XCB xcb)
        pkg_check_modules(XCB_EWMH xcb-ewmh)
        pkg_check_modules(XCB_SCREENSAVER xcb-screensaver)

        if(XCB_SCREENSAVER_FOUND AND (Qt5X11Extras_FOUND OR QT_VERSION_MAJOR VERSION_EQUAL 6))
            target_sources(
                ckb-next
                    PRIVATE
                        xcb/xcbidletimer.cpp
                        xcb/xcbidletimer.h
            )
            target_compile_definitions(
            ckb-next
                PRIVATE
                    USE_XCB_SCREENSAVER)
        else()
            message(FATAL_ERROR "XCB-Screensaver or Qt5 X11Extras was not found. Either install them or pass -DUSE_XCB=0 to disable idle detection.")
        endif()
        if(XCB_FOUND AND (Qt5X11Extras_FOUND OR QT_VERSION_MAJOR VERSION_EQUAL 6))
            target_compile_definitions(
            ckb-next
                PRIVATE
                    USE_XCB)
        else()
            message(FATAL_ERROR "XCB or Qt5 X11Extras was not found. Either install them or pass -DUSE_XCB=0 to disable idle detection.")
        endif()
        if(XCB_EWMH_FOUND)
            target_compile_definitions(
            ckb-next
                PRIVATE
                    USE_XCB_EWMH)

            target_sources(
            ckb-next
                PRIVATE
                xcb/xwindowdetector.cpp)
            else()
                message(FATAL_ERROR "XCB-EWMH was not found. Either install it or pass -DUSE_XCB=0 to disable active window detection.")
        endif()
    endif()

    target_sources(
        ckb-next
            PRIVATE
              media_linux.cpp)

    if(USE_DBUS_MENU)
        if(QT_VERSION VERSION_LESS "6")
            find_package(dbusmenu-qt5 CONFIG)
            if(NOT dbusmenu-qt5_FOUND)
                message(FATAL_ERROR "dbusmenu-qt5 was not found. Either install it or pass -DUSE_DBUS_MENU=0 to fall back to the default Qt tray icon.")
            endif()
            target_link_libraries(
                ckb-next
                    PRIVATE
                    dbusmenu-qt${QT_VERSION_MAJOR}
            )
            include_directories(${dbusmenu-qt5_INCLUDE_DIRS})
        endif()

        target_link_libraries(
            ckb-next
                PRIVATE
                Qt${QT_VERSION_MAJOR}::DBus
        )
        target_compile_definitions(
        ckb-next
            PRIVATE
                USE_DBUS_MENU)

        if(QT_VERSION VERSION_GREATER_EQUAL "5.15")
            qt_add_dbus_adaptor(statusNotifierAutogen "kstatusnotifier/org.kde.StatusNotifierItem.xml"
                                "kstatusnotifier/kstatusnotifieritemdbus_p.h" KStatusNotifierItemDBus)
            qt_add_dbus_interface(statusNotifierAutogen "kstatusnotifier/org.kde.StatusNotifierWatcher.xml" statusnotifierwatcher_interface)
            qt_add_dbus_interface(statusNotifierAutogen "kstatusnotifier/org.freedesktop.Notifications.xml" notifications_interface)
        else()
            qt5_add_dbus_adaptor(statusNotifierAutogen "kstatusnotifier/org.kde.StatusNotifierItem.xml"
                                "kstatusnotifier/kstatusnotifieritemdbus_p.h" KStatusNotifierItemDBus)
            qt5_add_dbus_interface(statusNotifierAutogen "kstatusnotifier/org.kde.StatusNotifierWatcher.xml" statusnotifierwatcher_interface)
            qt5_add_dbus_interface(statusNotifierAutogen "kstatusnotifier/org.freedesktop.Notifications.xml" notifications_interface)
        endif()

        #set_property(SOURCE ${statusNotifierAutogen} PROPERTY SKIP_AUTOGEN ON)
        target_sources(
            ckb-next
                PRIVATE
                  kstatusnotifier/kstatusnotifieritem.cpp
                  kstatusnotifier/kstatusnotifieritemdbus_p.cpp
                  kstatusnotifier/kstatusnotifieritemdbus_p.h
                  kstatusnotifier/kstatusnotifieritem.h
                  kstatusnotifier/kstatusnotifieritemprivate_p.h
                  kstatusnotifier/debug_p.h
                  kstatusnotifier/config-kstatusnotifieritem.h
                  ckbsystemtrayicon.cpp
                  ${statusNotifierAutogen})
        if(QT_VERSION VERSION_GREATER_EQUAL "6")
            qt6_add_dbus_adaptor(canonicalDbusMenuAutogen "${CMAKE_CURRENT_SOURCE_DIR}/kstatusnotifier/libdbusmenu-qt/com.canonical.dbusmenu.xml"
                "${CMAKE_CURRENT_SOURCE_DIR}/kstatusnotifier/libdbusmenu-qt/dbusmenuexporterdbus_p.h"
                DBusMenuExporterDBus)

            #set_property(SOURCE ${canonicalDbusMenuAutogen} PROPERTY SKIP_AUTOGEN ON)
            target_sources(
                ckb-next
                    PRIVATE
                        kstatusnotifier/libdbusmenu-qt/dbusmenu_p.cpp
                        kstatusnotifier/libdbusmenu-qt/dbusmenuexporter.cpp
                        kstatusnotifier/libdbusmenu-qt/dbusmenuexporterdbus_p.cpp
                        kstatusnotifier/libdbusmenu-qt/dbusmenushortcut_p.cpp
                        kstatusnotifier/libdbusmenu-qt/dbusmenutypes_p.cpp
                        kstatusnotifier/libdbusmenu-qt/utils.cpp
                        kstatusnotifier/libdbusmenu-qt/dbusmenu_p.h
                        kstatusnotifier/libdbusmenu-qt/dbusmenuexporter.h
                        kstatusnotifier/libdbusmenu-qt/dbusmenuexporterdbus_p.h
                        kstatusnotifier/libdbusmenu-qt/dbusmenuexporterprivate_p.h
                        kstatusnotifier/libdbusmenu-qt/dbusmenushortcut_p.h
                        kstatusnotifier/libdbusmenu-qt/dbusmenutypes_p.h
                        kstatusnotifier/libdbusmenu-qt/utils_p.h
                        kstatusnotifier/libdbusmenu-qt/debug_p.h
                        ${canonicalDbusMenuAutogen}
            )
        endif()
    endif()

    target_sources(
        ckb-next
            PRIVATE
                wayland/waylandutils.cpp
                wayland/waylandutils.h
    )

    if (USE_WAYLAND)
        find_package(Wayland COMPONENTS Client REQUIRED)
        find_package(WaylandProtocols REQUIRED)
        find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS WaylandClient)

        if (QT_VERSION VERSION_GREATER_EQUAL "6.0.0")
            if (NOT EXISTS ${WaylandProtocols_DATADIR}/staging/ext-idle-notify/ext-idle-notify-v1.xml)
                message(FATAL_ERROR "Wayland ext-idle-notify-v1 protocol is not found. Either install them or pass -DUSE_WAYLAND=0 to disable idle detection.")
            endif ()

            if (Qt6_VERSION VERSION_GREATER_EQUAL "6.8.0")
                set(private_code_option  "PRIVATE_CODE")
            endif()

            qt_generate_wayland_protocol_client_sources(
               ckb-next
                   ${private_code_option}
                   FILES
                       ${WaylandProtocols_DATADIR}/staging/ext-idle-notify/ext-idle-notify-v1.xml
            )

            target_sources(
                ckb-next
                    PRIVATE
                        wayland/waylandidletimer.cpp
                        wayland/waylandidletimer.h
            )

            target_compile_definitions(
                ckb-next
                    PRIVATE
                        USE_WAYLAND_IDLE_NOTIFY
            )
        else()
            message(FATAL_ERROR "Wayland idle detection required Qt 6 or later. Either build with Qt 6 or pass -DUSE_WAYLAND=0 to disable idle detection.")
        endif()

        target_compile_definitions(
            ckb-next
                PRIVATE
                    USE_WAYLAND
        )

        target_link_libraries(
            ckb-next
                PRIVATE
                    Wayland::Client
                    Qt${QT_VERSION_MAJOR}::WaylandClient
        )

        if (Qt6_VERSION VERSION_LESS "6.5.0")
            target_link_libraries(
                ckb-next
                    PRIVATE
                        Qt${QT_VERSION_MAJOR}::GuiPrivate # For QPlatformNativeInterface. Qt 6.5 and later has QNativeInterface::QWaylandApplication.
            )
        endif ()
    endif ()
endif ()

if (MAC_LEGACY)
    message(WARNING "Building a legacy GUI")
    target_compile_definitions(
        ckb-next
            PRIVATE
                OS_MAC_LEGACY)
endif ()

# Manipulate auxiliary files

# Generate binary.qrc
if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/binary.qrc")
    message(STATUS "Generating binary.qrc")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/binary.qrc" "<RCC>\n<qresource prefix=\"/bin\">\n")
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/binary.qrc" "<file>ckb-next-key.gpg</file>\n")
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/binary.qrc" "</qresource>\n</RCC>\n")
    # Import GPG key
    message(STATUS "Importing ckb-next-key.gpg")
    configure_file(
      "${CMAKE_CURRENT_LIST_DIR}/resources/ckb-next-key.gpg"
      "${CMAKE_CURRENT_BINARY_DIR}/ckb-next-key.gpg"
        COPYONLY)
endif ()

# Import images and generate image.qrc
if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/image.qrc")
    message(STATUS "Generating image.qrc")
    file(GLOB qt_images RELATIVE "${CMAKE_CURRENT_LIST_DIR}/resources" "${CMAKE_CURRENT_LIST_DIR}/resources/*.png")
    list(APPEND qt_images "lightning.svg" "lightning_disabled.svg")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/image.qrc" "<RCC>\n<qresource prefix=\"/img\">\n")
    foreach (qt_image IN LISTS qt_images)
        message(STATUS "Importing ${qt_image}")
        file(COPY "${CMAKE_CURRENT_LIST_DIR}/resources/${qt_image}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
        file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/image.qrc" "<file>${qt_image}</file>\n")
    endforeach ()
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/image.qrc" "</qresource>\n</RCC>\n")
endif ()

find_package(Qt${QT_VERSION_MAJOR}LinguistTools CONFIG REQUIRED)

# Generate translation.qrc
# Note that the qm files haven't been generated yet
if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/translation.qrc")
    file(GLOB qt_translations RELATIVE "${CMAKE_CURRENT_LIST_DIR}/resources/translations" "${CMAKE_CURRENT_LIST_DIR}/resources/translations/*.ts")
    message(STATUS "Generating translation.qrc")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/translation.qrc" "<RCC>\n<qresource prefix=\"/translations\">\n")
    foreach (qt_ts_translation IN LISTS qt_translations)
        string(REPLACE ".ts" ".qm" qt_qm_translation ${qt_ts_translation})
        message(STATUS "Importing ${qt_qm_translation}")
        file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/translation.qrc" "<file>${qt_qm_translation}</file>\n")
    endforeach ()
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/translation.qrc" "</qresource>\n</RCC>\n")
endif ()

# Generate binary translations and update ts files if needed
file(GLOB qt_translations_rel RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "${CMAKE_CURRENT_LIST_DIR}/resources/translations/*.ts")
if (QT_VERSION VERSION_GREATER_EQUAL "5.15")
    qt_add_translation(QM_FILES "${qt_translations_rel}")
else ()
    qt5_add_translation(QM_FILES "${qt_translations_rel}")
endif ()
add_custom_target(ckb-next-lrelease SOURCES "${QM_FILES}")

# Simple target to call lupdate on demand
add_custom_target(lupdate
    COMMAND Qt${QT_VERSION_MAJOR}::lupdate "." "-ts" ${qt_translations_rel}
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")

if (MACOS)
    # Import macOS icon
    if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/ckb-next.icns")
        message(STATUS "Importing ckb-next.icns")
        configure_file(
          "${ckb-next_SOURCE_DIR}/macos/ckb-next.icns"
          "${CMAKE_CURRENT_BINARY_DIR}/ckb-next.icns"
            COPYONLY)
    endif ()

    # Configure plist
    set(MACOSX_BUNDLE_EXECUTABLE_NAME "ckb-next")
    set(MACOSX_BUNDLE_INFO_STRING "ckb-next - version ${ckb-next_VERSION}")
    set(MACOSX_BUNDLE_ICON_FILE "ckb-next.icns")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.ckb-next")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${ckb-next_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_NAME "ckb-next_v${ckb-next_VERSION_MAJOR}.${ckb-next_VERSION_MINOR}.${ckb-next_VERSION_PATCH}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${ckb-next_VERSION_MAJOR}.${ckb-next_VERSION_MINOR}.${ckb-next_VERSION_PATCH}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${ckb-next_VERSION}")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2014-2016 ccMSC\nCopyright (c) 2017-2019 ckb-next Development Team")

    # Generate and import Info.plist
    message(STATUS "Generating and importing org.ckb-next.info.plist")
    configure_file(
      "${CMAKE_SOURCE_DIR}/macos/org.ckb-next.info.plist.in"
      "${CMAKE_CURRENT_BINARY_DIR}/org.ckb-next.info.plist"
        @ONLY)

    # Import application plist
    message(STATUS "Importing org.ckb-next.plist")
    configure_file(
      "${CMAKE_SOURCE_DIR}/macos/org.ckb-next.plist"
      "${CMAKE_CURRENT_BINARY_DIR}/org.ckb-next.plist"
        COPYONLY)

    if (NOT TEXT_QRC_STRING)
        set(TEXT_QRC_STRING "<file>org.ckb-next.plist</file>\n")
    endif ()

elseif (LINUX)
    if (NOT TEXT_QRC_STRING)
        set(TEXT_QRC_STRING "<file>ckb-next.autostart.desktop</file>\n")
    endif ()

    # Generate and import desktop files
    message(STATUS "Generating and importing ckb-next.desktop")
    configure_file(
      "${CMAKE_SOURCE_DIR}/linux/ckb-next.desktop.in"
      "${CMAKE_CURRENT_BINARY_DIR}/ckb-next.desktop"
        @ONLY)
    message(STATUS "Generating and importing ckb-next.autostart.desktop")
    configure_file(
      "${CMAKE_SOURCE_DIR}/linux/ckb-next.autostart.desktop.in"
      "${CMAKE_CURRENT_BINARY_DIR}/ckb-next.autostart.desktop"
        @ONLY)
endif ()

# Generate text.qrc
if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/text.qrc")
    # Import demo profile
    message(STATUS "Importing demo profiles and generating text.qrc")
    file(GLOB gui_demos RELATIVE "${CMAKE_CURRENT_LIST_DIR}/resources/demos" "${CMAKE_CURRENT_LIST_DIR}/resources/demos/*")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/text.qrc" "<RCC>\n<qresource prefix=\"/txt\">\n")
    foreach (gui_demo IN LISTS gui_demos)
        message(STATUS "Importing ${gui_demo}")
        file(COPY "${CMAKE_CURRENT_LIST_DIR}/resources/demos/${gui_demo}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
        file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/text.qrc" "<file>${gui_demo}</file>\n")
    endforeach ()
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/text.qrc" "${TEXT_QRC_STRING}")
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/text.qrc" "</qresource>\n</RCC>\n")
endif ()

# Enhance target's source files based on auxiliary files
if (MACOS OR LINUX)
    target_sources(
        ckb-next
            PRIVATE
              "${CMAKE_CURRENT_BINARY_DIR}/binary.qrc"
              "${CMAKE_CURRENT_BINARY_DIR}/text.qrc"
              "${CMAKE_CURRENT_BINARY_DIR}/image.qrc"
              "${CMAKE_CURRENT_BINARY_DIR}/translation.qrc")
endif ()

# Declare target's include paths
if (MACOS OR LINUX)
    target_include_directories(
        ckb-next
            PRIVATE
              "${CMAKE_CURRENT_SOURCE_DIR}"
              "${CMAKE_CURRENT_BINARY_DIR}")
endif ()

target_include_directories(
    ckb-next
        SYSTEM
        PRIVATE
          "${QuaZip_INCLUDE_DIR}")

# Declare target's link dependencies
if (MACOS OR LINUX)
    target_link_libraries(
        ckb-next
            PRIVATE
                m
                Threads::Threads
                Qt${QT_VERSION_MAJOR}::Core
                Qt${QT_VERSION_MAJOR}::Network
                Qt${QT_VERSION_MAJOR}::Widgets
                Qt${QT_VERSION_MAJOR}::Gui
                ZLIB::ZLIB)
if (QT_VERSION VERSION_GREATER_EQUAL "6")
    target_link_libraries(
        ckb-next
            PRIVATE
                Qt6::OpenGLWidgets)
endif ()

endif ()
if(MACOS)
    target_link_libraries(
        ckb-next
            PRIVATE
              "${COREAUDIO_LIBRARY}"
              "${FOUNDATION_LIBRARY}"
              "${AUDIOTOOLBOX_LIBRARY}")
elseif (LINUX)
    if (QT_VERSION_MAJOR VERSION_EQUAL "5")
        target_link_libraries(
            ckb-next
                PRIVATE
                  Qt5::X11Extras)
    endif ()

    target_link_libraries(
        ckb-next
            PRIVATE
              "${XCB_EWMH_LIBRARIES}"
              "${XCB_SCREENSAVER_LIBRARIES}")

    if (PULSEAUDIO_FOUND)
        target_link_libraries(
            ckb-next
                PRIVATE
                  "${PULSEAUDIO_LIBRARIES}")
    else()
        message(WARNING "PulseAudio not found. Mute indicator will not function.")
    endif ()
endif()

target_link_libraries(
    ckb-next
        PRIVATE
        "${QuaZip_LIBRARIES}")

# Target compile features and options
if (QT_VERSION_MAJOR VERSION_LESS "6")
    set_target_properties(
        ckb-next
            PROPERTIES
              CXX_STANDARD 11)
else ()
    set_target_properties(
        ckb-next
            PROPERTIES
              CXX_STANDARD 17)
endif()

target_compile_options(
    ckb-next
        PRIVATE
          "${CKB_NEXT_COMMON_COMPILE_FLAGS}"
          "${CKB_NEXT_EXTRA_CXX_FLAGS}")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(
        ckb-next
            PRIVATE
              "-DQT_DEPRECATED_WARNINGS")
endif ()

# Add sanitizers after all target information is known
add_sanitizers(ckb-next)

# Declare target's installation paths
if (MACOS)
    install(
        TARGETS ckb-next
        DESTINATION ".")
elseif (LINUX)
    install(
        TARGETS ckb-next
        DESTINATION "bin")
endif ()

# Declare target's auxiliary files installation paths
if (MACOS)
    # Install Info.plist file inside the bundle
    set_target_properties(
        ckb-next
            PROPERTIES
              MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/org.ckb-next.info.plist")

    # Install icon file
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/ckb-next.icns"
        DESTINATION "ckb-next.app/Contents/Resources"
        PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ)

    # Install daemon restart script
    install(
      PROGRAMS "${PROJECT_SOURCE_DIR}/macos/daemon-restart.sh"
      DESTINATION "ckb-next.app/Contents/Resources"
      PERMISSIONS
      OWNER_READ OWNER_WRITE OWNER_EXECUTE
      GROUP_READ GROUP_EXECUTE
      WORLD_READ WORLD_EXECUTE)
elseif (LINUX)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/linux/icons/"
        DESTINATION "share/icons/hicolor"
        FILE_PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ
        DIRECTORY_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)

    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/ckb-next.png"
        "${CMAKE_CURRENT_BINARY_DIR}/ckb-next-monochrome.png"
        DESTINATION "share/icons/hicolor/512x512/apps"
        PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ)

    if (SAFE_INSTALL)
        # We need to use file(WRITE) because TOUCH was added way too late
        # xdg-icon-resource does this to force a cache update
        # Should be enough for all modern desktops
        install(
            CODE
            "file(WRITE \"${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/.xdg-icon-resource-dummy\" \"\")
            file(REMOVE \"${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/.xdg-icon-resource-dummy\")")
    endif ()

    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/ckb-next.desktop"
        DESTINATION "share/applications"
        PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ)
endif ()

# Deploy QT5
# NOTE: Must be executed after all install() commands
if (MACOS)
    set(dirs "")
    if (CMAKE_PREFIX_PATH)
        foreach (dir "${CMAKE_PREFIX_PATH}")
            list(APPEND dirs "${dir}/bin" "${dir}/lib")
        endforeach ()
    endif ()

    set(plugins "")
    # since Qt 5.10 'macintosh' widget style is a separate plugin
    if ((Qt5Core_VERSION VERSION_GREATER 5.10.0) OR (Qt5Core_VERSION VERSION_EQUAL 5.10.0))
        list(APPEND plugins "${qt5_path}/plugins/styles/libqmacstyle.dylib")
    endif ()

    include(InstallRequiredSystemLibraries)
    include(DeployQt5)
    install_qt5_executable("ckb-next.app" "${plugins}" "" "${dirs}" "")
endif ()
