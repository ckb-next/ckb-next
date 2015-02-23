TEMPLATE = app
TARGET = ckb-ripple

CONFIG   += debug_and_release
CONFIG   += console
CONFIG   -= qt app_bundle
QT       -= core gui
LIBS     -= -lQtGui -lQtCore

QMAKE_CFLAGS += -std=c99

QMAKE_MAC_SDK = macosx10.10

macx {
    DESTDIR = $$PWD/../../ckb.app/Contents/Resources/ckb-animations
} else {
    DESTDIR = $$PWD/../../bin/ckb-animations
}

SOURCES += \
    main.c
