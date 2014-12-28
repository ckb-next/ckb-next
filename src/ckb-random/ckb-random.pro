QT       -= gui

QMAKE_CFLAGS += -std=c99

QMAKE_MAC_SDK = macosx10.10

TARGET = ckb-random
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += debug_and_release

TEMPLATE = app

DESTDIR = $$PWD/../../bin/animations

SOURCES += \
	main.c
