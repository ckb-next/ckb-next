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
    find_package(Qt5 5.2.0 COMPONENTS Core Widgets Network Gui REQUIRED)
    find_package(ZLIB REQUIRED)
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
    if ((Qt5Core_VERSION VERSION_LESS 5.7.0) OR USE_APPINDICATOR_OVERRIDE)
        find_package(Appindicator)
        if (Appindicator_FOUND)
            target_compile_definitions(
                ckb-next
                    PRIVATE
                        USE_LIBAPPINDICATOR)
        endif ()
    endif ()
    find_package(X11 REQUIRED)
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
              macroreader.cpp
              main.cpp
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
    find_package(PulseAudio)
    if (${PULSEAUDIO_FOUND})
        target_compile_definitions(
            ckb-next
                PRIVATE
                    USE_LIBPULSE)
    endif ()

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
      "${CMAKE_CURRENT_BINARY_DIR}/ckb-next-key.gpg"
        COPYONLY)
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
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2014-2016 ccMSC\nCopyright (c) 2017-2018 ckb-next Development Team")

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
    message(STATUS "Importing demoprofile.conf")
    configure_file(
      "${CMAKE_CURRENT_LIST_DIR}/resources/demoprofile.conf"
      "${CMAKE_CURRENT_BINARY_DIR}/demoprofile.conf"
        COPYONLY)
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
              "${CMAKE_CURRENT_SOURCE_DIR}"
              "${CMAKE_CURRENT_BINARY_DIR}")
endif ()
if (MACOS)
elseif (LINUX)
    target_include_directories(
        ckb-next
            PRIVATE
              "${X11_INCLUDE_DIR}"
              "${Appindicator_INCLUDE_DIRS}")
endif ()

if (NOT WITH_SHIPPED_QUAZIP)
    target_include_directories(
        ckb-next
            PRIVATE
              "${QuaZip_INCLUDE_DIR}") # TODO: export QuaZip target and inherit dirs
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
              ZLIB::ZLIB)
endif ()
if(MACOS)
    target_link_libraries(
        ckb-next
            PRIVATE
              "${COREAUDIO_LIBRARY}"
              "${FOUNDATION_LIBRARY}"
              "${AUDIOTOOLBOX_LIBRARY}")
elseif (LINUX)
    if(${PULSEAUDIO_FOUND})
        target_link_libraries(
            ckb-next
                PRIVATE
                  "${X11_LIBRARIES}"
                  "${Appindicator_LIBRARIES}"
                  "${PULSEAUDIO_LIBRARIES}")
    else()
        target_link_libraries(
            ckb-next
                PRIVATE
                  "${X11_LIBRARIES}"
                  "${Appindicator_LIBRARIES}")
    endif()
endif()

if (WITH_SHIPPED_QUAZIP)
    target_link_libraries(
        ckb-next
            PRIVATE
              QuaZip::quazip5)
else ()
    target_link_libraries(
        ckb-next
            PRIVATE
            "${QuaZip_LIBRARIES}")
endif ()

# Target compile features and options
set_target_properties(
    ckb-next
        PROPERTIES
          CXX_STANDARD 11)

target_compile_options(
    ckb-next
        PRIVATE
          "${Appindicator_DEFINITIONS}"
          "${CKB_NEXT_COMMON_COMPILE_FLAGS}")

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
elseif (LINUX)
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/ckb-next.desktop"
        DESTINATION "share/applications"
        PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ)

    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/ckb-next.png"
        DESTINATION "share/icons/hicolor/512x512/apps"
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
    if (Qt5Core_VERSION VERSION_GREATER_EQUAL 5.10.0)
        list(APPEND plugins "${qt5_path}/plugins/styles/libqmacstyle.dylib")
    endif ()

    include(InstallRequiredSystemLibraries)
    include(DeployQt5)
    install_qt5_executable("ckb-next.app" "${plugins}" "" "${dirs}" "")
endif ()
