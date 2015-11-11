TEMPLATE = app
TARGET = ckb-daemon

macx {
    DESTDIR = $$PWD/../../ckb.app/Contents/Resources
} else {
    DESTDIR = $$PWD/../../bin
}

macx {
    LIBS = -framework CoreFoundation -framework CoreGraphics -framework IOKit -liconv
} else {
    LIBS = -lpthread -ludev
}

QMAKE_CFLAGS  = -std=gnu99 -Wno-unused-parameter -Werror=all
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9

# Minimal build - remove Qt defaults
CONFIG   = debug_and_release
QT       =

CKB_VERSION_STR = `cat $$PWD/../../VERSION`
DEFINES += CKB_VERSION_STR="\\\"$$CKB_VERSION_STR\\\""

SOURCES += \
    device.c \
    devnode.c \
    input_linux.c \
    input_mac.c \
    input.c \
    main.c \
    notify.c \
    usb_linux.c \
    usb_mac.c \
    usb.c \
    firmware.c \
    profile.c \
    extra_mac.c \
    keymap.c \
    command.c \
    device_vtable.c \
    device_keyboard.c \
    device_mouse.c \
    led_keyboard.c \
    led.c \
    led_mouse.c \
    input_mac_mouse.c \
    profile_keyboard.c \
    dpi.c \
    profile_mouse.c

HEADERS += \
    device.h \
    devnode.h \
    includes.h \
    input.h \
    led.h \
    notify.h \
    os.h \
    usb.h \
    firmware.h \
    profile.h \
    command.h \
    keymap.h \
    keymap_mac.h \
    structures.h \
    dpi.h
