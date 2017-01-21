set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter") # <-- fix this
set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -Wno-unused-parameter)

#set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS} -g -O0 -Wall)
#set(CMAKE_C_FLAGS_DEBUG "")
#set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS} -03 -Wno-unused-parameter)
#set(CMAKE_C_FLAGS_RELEASE "")

message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: " ${CMAKE_CXX_FLAGS_DEBUG})
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: " ${CMAKE_CXX_FLAGS_RELEASE})

# Qt automagic things
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# set apple search directories
if (APPLE)
    message(STATUS "Building for macOS")
    message(STATUS "CMAKE_PREFIX_PATH before brew: " ${CMAKE_PREFIX_PATH})
    execute_process(COMMAND brew --prefix qt5
            RESULT_VARIABLE BREW_AND_QT_FOUND
            OUTPUT_VARIABLE CMAKE_PREFIX_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT ${BREW_AND_QT_FOUND} STREQUAL "0")
        message(FATAL_ERROR "Homebrew is not installed!")
    endif ()
    message(STATUS "CMAKE_PREFIX_PATH after brew: " ${CMAKE_PREFIX_PATH})
endif ()

# look for qt
find_package(Qt5 COMPONENTS Core Widgets Network REQUIRED)
if (Qt5Core_FOUND)
    if (Qt5Core_VERSION VERSION_LESS 5.6.0)
        message(FATAL_ERROR "Minimum supported Qt5 version is 5.6!")
    else ()
        message(STATUS "Found Qt ${Qt5Core_VERSION_STRING}")
    endif ()
endif ()
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Network_INCLUDE_DIRS})

# look for zlib
find_package(ZLIB REQUIRED)
message(STATUS "Found Zlib ${ZLIB_VERSION_STRING}")
include_directories(${ZLIB_INCLUDE_DIRS})

# look for apple frameworks
if (APPLE)
    find_library(COREAUDIO_LIBRARY CoreAudio)
    message(STATUS "Found CoreAudio")
    find_library(FOUNDATION_LIBRARY Foundation)
    message(STATUS "Found Foundation")
    find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
    message(STATUS "Found AudioToolbox")
    mark_as_advanced(
            COREAUDIO_LIBRARY
            FOUNDATION_LIBRARY
            AUDIOTOOLBOX_LIBRARY
    )
endif ()

# bundle settings
if (APPLE)
    set(MACOSX_BUNDLE_DISPLAY_NAME "CKB Next")
    set(MACOSX_BUNDLE_EXECUTABLE_NAME "ckb-next")
    set(MACOSX_BUNDLE_ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/ckb-logo.icns)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.next.ckb")
    set(MACOSX_BUNDLE_BUNDLE_NAME "ckb")
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_COPYRIGHT "2014-2016: ccMSC, 2017: the ckb-next dev team")
endif ()

# The order is taken from qmake, could be improved later
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

add_definitions(-DQUAZIP_STATIC)

if (APPLE)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${HEADERS} ${SOURCES} ${RESOURCES} ${OBJC_SOURCES})
    target_link_libraries(${PROJECT_NAME} ${LIBS} ${APPLE_LIBS})
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in)
endif ()

#set(CHECK_X11 "pkg-config --exists x11")
#set(APP_IND "pkg-config --exists appindicator-0.1")
#
#execute_process(COMMAND ${CHECK_X11} RESULT_VARIABLE RESULT)
#if ("${RESULT}" STREQUAL "0")
#    message("Using X11")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lX11")
#    add_definitions(-DUSE_LIBX11)
#endif ("${RESULT}" STREQUAL "0")
#
## todo: set this up.
#execute_process(COMMAND ${APP_IND} RESULT_VARIABLE RESULT)
#if ("${RESULT}" STREQUAL "0")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lX11")
#    add_definitions(-DUSE_LIBX11)
#endif ("${RESULT}" STREQUAL "0")
#
#execute_process(COMMAND cat ${CMAKE_SOURCE_DIR}/VERSION RESULT_VARIABLE CKB_VERSION_STR)
#add_definitions(-DCKB_VERSION_STR="\\\"${CKB_VERSION_STR}\\\"")
