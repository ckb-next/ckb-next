#
#        Copyright © 2017 Oleksii Vilchanskyi <alexvilchansky@yahoo.com>
#        
#        An inspiration on how to use DeployQt5 module was taken from OpenChemistry
#        project Avogadro 2, where David Cole and Markus D. Hanwell have contributed it.
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

# Declare the target
# Unfortunately, CMake doesn't allow doing this
# without specifying at least one source file, so here you have it
add_executable(ckb-next main.cpp)

# Declare we want macOS Bundle
if (MACOS)
    set_target_properties(
        ckb-next
            PROPERTIES
              MACOSX_BUNDLE TRUE)
endif ()

# Prepare external dependencies' paths
if (BREW_QT5)
    execute_process(
        COMMAND brew --prefix qt5
        RESULT_VARIABLE qt5_found
        OUTPUT_VARIABLE qt5_path
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT ("${qt5_found}" EQUAL 0))
        message(FATAL_ERROR "Homebrew's 'qt5' package is not installed.")
    else ()
        message(STATUS "Homebrew's Qt5 found: ${qt5_path}")
        list(APPEND CMAKE_PREFIX_PATH "${qt5_path}")
    endif ()
endif ()

# Find external dependencies
if (MACOS OR LINUX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    find_package(Qt5 5.2.0 REQUIRED COMPONENTS Core Widgets Network Gui)
    find_package(ZLIB REQUIRED)
    # TODO: rewrite module
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
              animscript.cpp
              animsettingdialog.cpp
              autorun.cpp
              ckbsettings.cpp
              ckbsettingswriter.cpp
              colorbutton.cpp
              colormap.cpp
              extrasettingswidget.cpp
              fwupgradedialog.cpp
              gradientbutton.cpp
              gradientdialog.cpp
              gradientdialogwidget.cpp
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
              kbperf.cpp
              kbprofile.cpp
              kbprofiledialog.cpp
              kbwidget.cpp
              keyaction.cpp
              keymap.cpp
              keywidget.cpp
              kperfwidget.cpp
              layoutdialog.cpp
              macroreader.cpp
              mainwindow.cpp
              modeselectdialog.cpp
              mperfwidget.cpp
              rebindwidget.cpp
              rlistwidget.cpp
              settingswidget.cpp
              animadddialog.h
              animscript.h
              animsettingdialog.h
              autorun.h
              ckbsettings.h
              ckbsettingswriter.h
              colorbutton.h
              colormap.h
              extrasettingswidget.h
              fwupgradedialog.h
              gradientbutton.h
              gradientdialog.h
              gradientdialogwidget.h
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
              kbperf.h
              kbprofiledialog.h
              kbprofile.h
              kbwidget.h
              keyaction.h
              keymap.h
              keywidget.h
              kperfwidget.h
              layoutdialog.h
              macroreader.h
              mainwindow.h
              media.h
              modeselectdialog.h
              mperfwidget.h
              rebindwidget.h
              rlistwidget.h
              settingswidget.h
              animadddialog.ui
              animsettingdialog.ui
              extrasettingswidget.ui
              fwupgradedialog.ui
              gradientdialog.ui
              kbanimwidget.ui
              kbbindwidget.ui
              kblightwidget.ui
              kbprofiledialog.ui
              kbwidget.ui
              kperfwidget.ui
              layoutdialog.ui
              mainwindow.ui
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
    target_sources(
        ckb-next
            PRIVATE
              media_linux.cpp)
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
        "${CMAKE_CURRENT_BINARY_DIR}/ckb-next-key.gpg" COPYONLY)
endif ()

# Import images and generate image.qrc
if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/image.qrc")
    message(STATUS "Generating image.qrc")
    file(GLOB qt_images RELATIVE "${CMAKE_CURRENT_LIST_DIR}/resources" "${CMAKE_CURRENT_LIST_DIR}/resources/*.png")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/image.qrc" "<RCC>\n<qresource prefix=\"/img\">\n")
    foreach (qt_image IN LISTS qt_images)
        message(STATUS "Importing ${qt_image}")
        file(COPY "${CMAKE_CURRENT_LIST_DIR}/resources/${qt_image}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
        file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/image.qrc" "<file>${qt_image}</file>\n")
    endforeach ()
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/image.qrc" "</qresource>\n</RCC>\n")
endif ()

if (MACOS)
    # Import macOS icon
    if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/ckb-next.icns")
        message(STATUS "Importing ckb-next.icns")
        configure_file(
            "${CMAKE_SOURCE_DIR}/macos/ckb-next.icns"
            "${CMAKE_CURRENT_BINARY_DIR}/ckb-next.icns" COPYONLY)
    endif ()

    # Configure plist
    set(MACOSX_BUNDLE_HELP_FILE "Help") # FIXME: provide Help.html
    set(MACOSX_BUNDLE_EXECUTABLE_NAME "ckb-next")
    set(MACOSX_BUNDLE_INFO_STRING "ckb-next - version ${ckb-next_VERSION}")
    set(MACOSX_BUNDLE_ICON_FILE "ckb-next.icns")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.next.ckb")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${ckb-next_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_NAME "ckb-next_v${ckb-next_VERSION_MAJOR}.${ckb-next_VERSION_MINOR}.${ckb-next_VERSION_PATCH}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${ckb-next_VERSION_MAJOR}.${ckb-next_VERSION_MINOR}.${ckb-next_VERSION_PATCH}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${ckb-next_VERSION}")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright © 2014-2016 ccMSC\nCopyright © 2017 ckb-next dev. team")

    # Generate and import plist
    message(STATUS "Generating and importing org.next.ckb.plist")
    configure_file(
        "${CMAKE_SOURCE_DIR}/macos/org.next.ckb.plist.in"
        "${CMAKE_CURRENT_BINARY_DIR}/org.next.ckb.plist")

    if (NOT TEXT_QRC_STRING)
        set(TEXT_QRC_STRING "<file>org.next.ckb.plist</file>\n")
    endif ()

elseif (LINUX)
    if (NOT TEXT_QRC_STRING)
        set(TEXT_QRC_STRING "<file>ckb-next.desktop</file>\n")
    endif ()

    # Generate and import desktop file
    message(STATUS "Generating and importing ckb-next.desktop")
    configure_file(
      "${CMAKE_SOURCE_DIR}/linux/ckb-next.desktop.in"
      "${CMAKE_CURRENT_BINARY_DIR}/ckb-next.desktop")
endif ()

# Generate text.qrc
if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/text.qrc")
    # Import demo profile
    message(STATUS "Importing demoprofile.conf")
    configure_file(
      "${CMAKE_CURRENT_LIST_DIR}/resources/demoprofile.conf"
      "${CMAKE_CURRENT_BINARY_DIR}/demoprofile.conf" COPYONLY)
    message(STATUS "Generating text.qrc")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/text.qrc" "<RCC>\n<qresource prefix=\"/txt\">\n")
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/text.qrc" "<file>demoprofile.conf</file>\n")
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
              "${CMAKE_CURRENT_BINARY_DIR}/image.qrc")
endif ()

# Declare target's include paths
if (MACOS OR LINUX)
    target_include_directories(
        ckb-next
            PRIVATE
              "${CMAKE_CURRENT_BINARY_DIR}"
              "${CMAKE_CURRENT_SOURCE_DIR}"
              "${QUAZIP_INCLUDE_DIR}") # TODO: export QuaZip target and inherit dirs
endif ()

# Declare target's link dependencies
if (MACOS OR LINUX)
    target_link_libraries(
        ckb-next
            PRIVATE
              m
              Threads::Threads
              Qt5::Core
              Qt5::Network
              Qt5::Widgets
              Qt5::Gui
              ZLIB::ZLIB
              "${QUAZIP_LIBRARIES}")
endif ()
if(MACOS)
    target_link_libraries(
        ckb-next
            PRIVATE
              "${COREAUDIO_LIBRARY}"
              "${FOUNDATION_LIBRARY}"
              "${AUDIOTOOLBOX_LIBRARY}")
elseif (LINUX)
endif()

# Target compile features and options
set_target_properties(
    ckb-next
        PROPERTIES
          CXX_STANDARD 11)

if (CXX_COMPILER_SUPPORTS_-Og)
    set(opt_lvl "-Og")
else ()
    set(opt_lvl "-O0")
endif ()

target_compile_options(
    ckb-next
        PRIVATE
          -fsigned-char
          -Wall
          -Wextra
          -Winit-self
          $<$<CONFIG:Debug>:-Wfloat-equal>
          $<$<CONFIG:Debug>:-Wundef>
          $<$<CONFIG:Debug>:-Wshadow>
          $<$<CONFIG:Debug>:-Wpointer-arith>
          $<$<CONFIG:Debug>:-Wcast-align>
          $<$<CONFIG:Debug>:-Wstrict-overflow=5>
          $<$<CONFIG:Debug>:-Wwrite-strings>
          $<$<CONFIG:Debug>:-Wcast-qual>
          $<$<CONFIG:Debug>:-Wswitch-default>
          $<$<CONFIG:Debug>:-Wswitch-enum>
          $<$<CONFIG:Debug>:-Wconversion>
          $<$<CONFIG:Debug>:-Wformat=2>
          $<$<CONFIG:Debug>:-save-temps>
          $<$<CONFIG:Debug>:${opt_lvl}>
          $<$<CONFIG:Release>:-Wno-unused-parameter>) # FIXME: cast out unused parameters

# Add sanitizers after all target information is known
add_sanitizers(ckb-next)

# Make sure ckb GUI will not autostart and interfere with ckb-next GUI
if (SAFE_INSTALL)
    set(autostart_paths
            "$ENV{HOME}/.config/autostart/ckb.desktop"
            "$ENV{HOME}/Library/LaunchAgents/com.ckb.ckb.plist")
    foreach (autostart_path IN LISTS autostart_paths)
        if (EXISTS "${autostart_path}")
            message(STATUS "ckb GUI autostart file detected")
            install(CODE "message(STATUS \"Removing ${autostart_path}\")")
            install(CODE "execute_process(COMMAND rm ${autostart_path})")
        endif ()
    endforeach ()
endif()

# Declare target's installation paths
if (MACOS)
    install(
        TARGETS ckb-next
        DESTINATION ".") # prefix is /Applications by default, therefore "."
elseif (LINUX)
    install(
        TARGETS ckb-next
        DESTINATION "bin")
endif ()

# Declare target's auxiliary files installation paths
if (MACOS)
    # Install plist file inside the bundle
    set_target_properties(
        ckb-next
            PROPERTIES
              MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/org.next.ckb.plist")

    # Install icon file and help page inside the bundle
    set_source_files_properties(
        ckb-next.icns
        Help.html # FIXME: generate
            PROPERTIES
              MACOSX_PACKAGE_LOCATION Resources)
elseif (LINUX)
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/ckb-next.desktop"
        DESTINATION "share/applications"
        PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ)

    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/ckb-next-logo.png"
        DESTINATION "share/icons/hicolor/512x512/apps"
        PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ)
endif ()

# Deploy QT5
# NOTE: Must be executed after all install() commands
if (MACOS)
    get_target_property(output_name ckb-next OUTPUT_NAME)
    if (output_name)
        set(app "${output_name}.app")
    else ()
        set(app "ckb-next.app")
    endif ()

    set(dirs "")
    if (CMAKE_PREFIX_PATH)
        foreach (dir "${CMAKE_PREFIX_PATH}")
            list(APPEND dirs "${dir}/bin" "${dir}/lib")
        endforeach ()
    endif ()

    include(InstallRequiredSystemLibraries)
    include(DeployQt5)
    install_qt5_executable("ckb-next.app" "" "" "${dirs}" "")
endif ()
