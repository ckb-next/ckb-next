set(CMAKE_INCLUDE_CURRENT_DIR ON)

#-------------------------------------------------------------------------------
# find Qt5
if (BREW_QT5)
    execute_process(COMMAND brew --prefix qt5
            RESULT_VARIABLE BREW_AND_QT_FOUND
            OUTPUT_VARIABLE CMAKE_PREFIX_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT ${BREW_AND_QT_FOUND} EQUAL 0)
        message(FATAL_ERROR "Homebrew is not installed!")
    endif ()
endif ()

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)
include_directories(
        ${Qt5Core_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Qt5Network_INCLUDE_DIRS}
)

# ------------------------------------------------------------------------------
# find Zlib
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

# ------------------------------------------------------------------------------
# find QuaZip, fallback to hardcoded
if (LOCAL_QUAZIP)
    find_package(QuaZip)

    if (ENFORCE_QUAZIP)
        if (NOT QUAZIP_FOUND)
            message(FATAL_ERROR "You enforced local QuaZip, but it's not found")
        endif ()
    endif ()

    if (QUAZIP_FOUND)
        include_directories(${QUAZIP_INCLUDE_DIR})
    endif ()

endif ()

if (NOT QUAZIP_FOUND)
    include_directories(quazip)

    set(QUAZIP_HEADERS
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
            )

    set(QUAZIP_SOURCES
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
            )

    add_definitions(-DQUAZIP_STATIC)
endif ()

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

if (APPLE)
    list(APPEND SOURCES media_mac.m)
endif ()

# ------------------------------------------------------------------------------
# Qt settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# ------------------------------------------------------------------------------
# mark libraries
set(LIBS
        Qt5::Core
        Qt5::Widgets
        Qt5::Network
        ZLIB::ZLIB
        )

if (QUAZIP_FOUND)
    list(APPEND LIBS ${QUAZIP_LIBRARIES})
else ()
    list(APPEND HEADERS ${QUAZIP_HEADERS})
    list(APPEND SOURCES ${QUAZIP_SOURCES})
endif ()

if (APPLE)
    list(APPEND LIBS
            ${COREAUDIO_LIBRARY}
            ${FOUNDATION_LIBRARY}
            ${AUDIOTOOLBOX_LIBRARY}
            )
endif ()

# ------------------------------------------------------------------------------
# set up targets
if (APPLE)
    set(MACOSX_BUNDLE_BUNDLE_NAME "ckb-next")
    set(MACOSX_BUNDLE_EXECUTABLE_NAME "ckb-next")
    set(MACOSX_BUNDLE_INFO_STRING "ckb-next - version ${ckb-next_VERSION}")
    set(MACOSX_BUNDLE_ICON_FILE "ckb-next.icns")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.ckb-next")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${ckb-next_VERSION}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${ckb-next_VERSION_MAJOR}.${ckb-next_VERSION_MINOR}.${ckb-next_VERSION_PATCH}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${ckb-next_VERSION}")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright © 2014-2016 ccMSC\nCopyright © 2017 ckb-next dev. team")
    configure_file("${CMAKE_CURRENT_LIST_DIR}/ckb-next.plist.in" "${CMAKE_CURRENT_LIST_DIR}/ckb-next.plist")
    #    configure_file("${CMAKE_CURRENT_LIST_DIR}/ckb-next.icns" "${CMAKE_BINARY_DIR}/ckb-next.icns")

    set_source_files_properties(ckb-next.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    list(APPEND SOURCES ckb-next.icns)
endif ()

add_executable(ckb-next ${HEADERS} ${SOURCES} ${QT_RESOURCES})

target_link_libraries(ckb-next ${LIBS})

if (APPLE)
    set_target_properties(ckb-next PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/ckb-next.plist"
            OUTPUT_NAME "${MACOSX_BUNDLE_BUNDLE_NAME}"
            )
endif ()

if (NOT LINUX_CUSTOM_INSTALL)
    install(TARGETS ckb-next
            RUNTIME DESTINATION "${INSTALL_RUNTIME_DIR}"
            BUNDLE DESTINATION .
            )
endif ()

if (LINUX AND NOT LINUX_CUSTOM_INSTALL)
    if (NOT EXISTS "/usr/bin/ckb-next")
        # symlink to /usr/bin
        install(CODE [[execute_process(COMMAND ln -sf "/opt/ckb-next/ckb-next" "/usr/bin/ckb-next")]])
    endif ()

    install(FILES "ckb-next.desktop"
            DESTINATION "/usr/share/applications"
            PERMISSIONS OWNER_READ OWNER_WRITE
            GROUP_READ
            WORLD_READ
            )
    install(FILES "ckb-next.png"
            DESTINATION "/usr/share/icons/hicolor/512x512/apps"
            PERMISSIONS OWNER_READ OWNER_WRITE
            GROUP_READ
            WORLD_READ
            )
endif ()

# Keep "add_subdirectory(lastinstall)" last
add_subdirectory(lastinstall)
