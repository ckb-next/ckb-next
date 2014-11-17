#-------------------------------------------------
#
# Project created by QtCreator 2014-11-15T21:21:10
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = ckb-daemon
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_MAC_SDK = macosx10.10

QMAKE_CFLAGS += -std=c99
CONFIG += warn_off

macx {
	LIBS += -framework IOKit -framework CoreFoundation -framework CoreGraphics -framework AppKit -liconv
} else {
	LIBS += -lpthread -ludev
}

OBJECTS_DIR = $$PWD/../../tmp-ckb-daemon
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
	usb.c

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
	usb.h

OBJECTIVE_SOURCES += \
	extra_mac.m
