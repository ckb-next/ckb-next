TARGET = ckb-daemon
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_MAC_SDK = macosx10.10

QMAKE_CFLAGS += -std=c99 -Wno-unused-parameter -Werror=implicit

macx {
	LIBS += -framework IOKit -framework CoreFoundation -framework CoreGraphics -framework AppKit -liconv
} else {
	LIBS += -lpthread -ludev
}

DESTDIR = $$PWD/../../bin

SOURCES += \
	device.c \
	devnode.c \
	input_linux.c \
	input_mac.c \
	input.c \
	keyboard_uk.c \
	keyboard_us.c \
	keyboard.c \
	led.c \
	main.c \
	notify.c \
	usb_linux.c \
	usb_mac.c \
	usb.c \
	firmware.c \
	profile.c

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
