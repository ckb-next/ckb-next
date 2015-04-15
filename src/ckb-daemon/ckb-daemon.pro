TEMPLATE = app
TARGET = ckb-daemon

macx {
    DESTDIR = $$PWD/../../ckb.app/Contents/Resources
} else {
    DESTDIR = $$PWD/../../bin
}

macx {
    LIBS = -framework CoreFoundation -framework IOKit -liconv
} else {
    LIBS = -lpthread -ludev
}

QMAKE_CFLAGS += -std=c99 -Wno-unused-parameter -Werror=implicit
QMAKE_MAC_SDK = macosx10.10

# Minimal build - remove Qt defaults
CONFIG   =
QT       =

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
    keyboard_fr.c \
    extra_mac.c \
    keyboard_es.c

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
