TEMPLATE = app
TARGET = ckb-daemon

CONFIG   += debug_and_release
CONFIG   += console
CONFIG   -= qt app_bundle
QT       -= core gui
LIBS     -= -lQtGui -lQtCore

QMAKE_MAC_SDK = macosx10.10

macx {
    DESTDIR = $$PWD/../../ckb.app/Contents/Resources
} else {
    DESTDIR = $$PWD/../../bin
}

QMAKE_CFLAGS += -std=c99 -Wno-unused-parameter -Werror=implicit

macx {
    LIBS += -framework AppKit -framework CoreFoundation -framework CoreGraphics -framework IOKit -liconv
} else {
    LIBS += -lpthread -ludev
}

CKB_VERSION_STR = `cat $$PWD/../../VERSION`
DEFINES += CKB_VERSION_STR="\\\"$$CKB_VERSION_STR\\\""

SOURCES += \
    device.c \
    devnode.c \
    input_linux.c \
    input_mac.c \
    input.c \
    keyboard_us.c \
    keyboard.c \
    led.c \
    main.c \
    notify.c \
    usb_linux.c \
    usb_mac.c \
    usb.c \
    firmware.c \
    profile.c \
    keyboard_se.c \
    keyboard_gb.c \
    keyboard_de.c \
    keyboard_fr.c

HEADERS += \
    device.h \
    devnode.h \
    includes.h \
    input.h \
    keyboard_mac.h \
    keyboard.h \
    led.h \
    notify.h \
    os.h \
    structures.h \
    usb.h \
    firmware.h \
    profile.h

OBJECTIVE_SOURCES += \
    extra_mac.m
