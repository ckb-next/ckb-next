set(CMAKE_INCLUDE_CURRENT_DIR ON)

#-------------------------------------------------------------------------------
# set Qt search directories on macOS
if (APPLE)
    if (NOT CMAKE_PREFIX_PATH)
        execute_process(COMMAND brew --prefix qt5
                RESULT_VARIABLE BREW_AND_QT_FOUND
                OUTPUT_VARIABLE CMAKE_PREFIX_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
        if (NOT ${BREW_AND_QT_FOUND} STREQUAL "0")
            message(FATAL_ERROR "Homebrew is not installed!")
        endif ()
    endif ()
endif ()

# ------------------------------------------------------------------------------
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

# ------------------------------------------------------------------------------
# find Zlib
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

# ------------------------------------------------------------------------------
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

# ------------------------------------------------------------------------------
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

set(QT_RESOURCES
        image.qrc
        text.qrc
        binary.qrc
        )

set(OBJC_SOURCES
        media_mac.m
        )

# ------------------------------------------------------------------------------
# add definitions to the source code
add_definitions(-DQUAZIP_STATIC)

# ------------------------------------------------------------------------------
# Qt settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

qt5_wrap_ui(${FORMS})
qt5_add_resources(${QT_RESOURCES})

# need to add current binary dir to include path for UI file headers
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# ------------------------------------------------------------------------------
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

    # mark additional files to put under /Resources on macOS
    set(MAC_RESOURCES
            ckb-logo.icns
            ${CMAKE_SOURCE_DIR}/osx/resources/LICENSE.html
            ${CMAKE_SOURCE_DIR}/README.md
            ${CMAKE_SOURCE_DIR}/BUILD.md
            ${CMAKE_BINARY_DIR}/ckb-next-daemon
            )
endif ()

# ------------------------------------------------------------------------------
# set up targets
if (APPLE)
    # set variables for creating a plist
    include(CkbNextPlist)

    add_executable(ckb-next MACOSX_BUNDLE ${HEADERS} ${SOURCES} ${QT_RESOURCES} ${OBJC_SOURCES} ckb-logo.icns)
    qt5_use_modules(ckb-next Core Widgets Network)
    target_link_libraries(ckb-next ${LIBS} ${APPLE_LIBS})
    set_target_properties(ckb-next PROPERTIES
#            OUTPUT_NAME "${MACOSX_BUNDLE_BUNDLE_NAME}"
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/CkbNext.plist"
            RESOURCE ${MAC_RESOURCES}
            )

    # deploy qt5 on macOS
    set(exe "ckb-next.app")
    set(dirs "")
    if (CMAKE_PREFIX_PATH)
        foreach (dir ${CMAKE_PREFIX_PATH})
            list(APPEND dirs "${dir}/bin" "${dir}/lib")
        endforeach ()
    endif ()

    include(InstallRequiredSystemLibraries)
    include(DeployQt5)
    install_qt5_executable("${exe}" "" "" "${dirs}" "")

endif ()

if (LINUX)
    # TODO finish this
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

    # Linux installation is supposed to be handled manually by package maintainers
endif ()
