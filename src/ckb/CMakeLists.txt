project(ckb-gui LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

#-------------------------------------------------------------------------------
# set Qt search directories on macOS
if (APPLE)
    execute_process(COMMAND brew --prefix qt5
            RESULT_VARIABLE BREW_AND_QT_FOUND
            OUTPUT_VARIABLE CMAKE_PREFIX_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT ${BREW_AND_QT_FOUND} STREQUAL "0")
        message(FATAL_ERROR "Homebrew is not installed!")
    endif ()
    message(STATUS "CMAKE_PREFIX_PATH: " ${CMAKE_PREFIX_PATH})
endif ()

#-------------------------------------------------------------------------------
# find Qt5
find_package(Qt5 COMPONENTS Core Widgets Network REQUIRED)
if (Qt5Core_FOUND)
    if (Qt5Core_VERSION VERSION_LESS 5.6.0)
        message(FATAL_ERROR "Minimum supported Qt5 version is 5.6!")
    endif ()
endif ()
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Network_INCLUDE_DIRS})

#-------------------------------------------------------------------------------
# Qt automagic things
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#-------------------------------------------------------------------------------
# find Zlib
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

#-------------------------------------------------------------------------------
# find Apple frameworks
if (APPLE)
    find_library(COREAUDIO_LIBRARY CoreAudio)
    find_library(FOUNDATION_LIBRARY Foundation)
    find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
    mark_as_advanced(
            COREAUDIO_LIBRARY
            FOUNDATION_LIBRARY
            AUDIOTOOLBOX_LIBRARY
    )
endif ()

#-------------------------------------------------------------------------------
# set up macOS Bundle
#if (APPLE)
#    set(MACOSX_BUNDLE_DISPLAY_NAME "CKB Next")
#    set(MACOSX_BUNDLE_EXECUTABLE_NAME "ckb-next")
#    set(MACOSX_BUNDLE_ICON_FILE "ckb-logo.icns")
#    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.next.ckb")
#    set(MACOSX_BUNDLE_BUNDLE_NAME "ckb")
#    set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")
#    set(MACOSX_BUNDLE_COPYRIGHT "2014-2016: ccMSC, 2017: the ckb-next dev team")
#endif ()

#-------------------------------------------------------------------------------
# mark all source files
set(HEADERS
        mainwindow.h
        kbwidget.h
        colorbutton.h
        settingswidget.h
        kblightwidget.h
        keymap.h
        media.h
        kblight.h
        kbprofile.h
        kbanimwidget.h
        animscript.h
        ckb-anim.h
        kbanim.h
        animadddialog.h
        animsettingdialog.h
        gradientbutton.h
        gradientdialog.h
        gradientdialogwidget.h
        kbmode.h
        kb.h
        rlistwidget.h
        kbprofiledialog.h
        keywidget.h
        kbbindwidget.h
        kbbind.h
        rebindwidget.h
        modeselectdialog.h
        quazip/crypt.h
        quazip/ioapi.h
        quazip/JlCompress.h
        quazip/quaadler32.h
        quazip/quachecksum32.h
        quazip/quacrc32.h
        quazip/quagzipfile.h
        quazip/quaziodevice.h
        quazip/quazip.h
        quazip/quazip_global.h
        quazip/quazipdir.h
        quazip/quazipfile.h
        quazip/quazipfileinfo.h
        quazip/quazipnewinfo.h
        quazip/unzip.h
        quazip/zip.h
        kbfirmware.h
        fwupgradedialog.h
        autorun.h
        ckbsettings.h
        kbperf.h
        ckbsettingswriter.h
        keyaction.h
        mperfwidget.h
        kperfwidget.h
        layoutdialog.h
        extrasettingswidget.h
        kbmanager.h
        colormap.h
        macroreader.h
        )

set(SOURCES
        main.cpp
        mainwindow.cpp
        kbwidget.cpp
        colorbutton.cpp
        settingswidget.cpp
        kblightwidget.cpp
        keymap.cpp
        media_linux.cpp
        kblight.cpp
        kbprofile.cpp
        kbanimwidget.cpp
        animscript.cpp
        kbanim.cpp
        animadddialog.cpp
        animsettingdialog.cpp
        gradientbutton.cpp
        gradientdialog.cpp
        gradientdialogwidget.cpp
        kbmode.cpp
        kb.cpp
        rlistwidget.cpp
        kbprofiledialog.cpp
        keywidget.cpp
        kbbindwidget.cpp
        kbbind.cpp
        rebindwidget.cpp
        modeselectdialog.cpp
        quazip/JlCompress.cpp
        quazip/qioapi.cpp
        quazip/quaadler32.cpp
        quazip/quacrc32.cpp
        quazip/quagzipfile.cpp
        quazip/quaziodevice.cpp
        quazip/quazip.cpp
        quazip/quazipdir.cpp
        quazip/quazipfile.cpp
        quazip/quazipfileinfo.cpp
        quazip/quazipnewinfo.cpp
        quazip/unzip.c
        quazip/zip.c
        kbfirmware.cpp
        fwupgradedialog.cpp
        autorun.cpp
        ckbsettings.cpp
        kbperf.cpp
        ckbsettingswriter.cpp
        keyaction.cpp
        mperfwidget.cpp
        kperfwidget.cpp
        layoutdialog.cpp
        extrasettingswidget.cpp
        kbmanager.cpp
        colormap.cpp
        macroreader.cpp
        )

set(FORMS
        mainwindow.ui
        kbwidget.ui
        settingswidget.ui
        kblightwidget.ui
        kbanimwidget.ui
        animadddialog.ui
        animsettingdialog.ui
        gradientdialog.ui
        kbprofiledialog.ui
        kbbindwidget.ui
        rebindwidget.ui
        modeselectdialog.ui
        fwupgradedialog.ui
        mperfwidget.ui
        kperfwidget.ui
        layoutdialog.ui
        extrasettingswidget.ui
        )

set(RESOURCES
        image.qrc
        text.qrc
        binary.qrc
        )

set(OBJC_SOURCES
        media_mac.m
        )

#-------------------------------------------------------------------------------
# mark libraries
set(LIBS
        Qt5::Core
        Qt5::Widgets
        Qt5::Network
        ZLIB::ZLIB
        )

if (APPLE)
    set(APPLE_LIBS
            ${COREAUDIO_LIBRARY}
            ${FOUNDATION_LIBRARY}
            ${AUDIOTOOLBOX_LIBRARY}
            )
endif ()

#-------------------------------------------------------------------------------
# add definitions to the source code
add_definitions(-DQUAZIP_STATIC)

#-------------------------------------------------------------------------------
# set up targets
if (APPLE)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${HEADERS} ${SOURCES} ${RESOURCES} ${OBJC_SOURCES})
    target_link_libraries(${PROJECT_NAME} ${LIBS} ${APPLE_LIBS})
#    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE_BUNDLE_NAME "ckb-next"
            MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION}"
            MACOSX_BUNDLE_COPYRIGHT "© 2014-2016 ccMSC, © 2017: the ckb-next dev team"
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.next.ckb"
            MACOSX_BUNDLE_ICON_FILE "ckb-logo.icns"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION}"
            )

    # install additional files for the bundle
#    install(DIRECTORY Resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Contents)
#    install(FILES ${MACOSX_BUNDLE_ICON_FILE} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Contents/Resources)
endif ()
