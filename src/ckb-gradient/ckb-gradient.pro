QT       -= gui

QMAKE_CFLAGS += -std=c99

QMAKE_MAC_SDK = macosx10.10

TARGET = ckb-gradient
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += debug_and_release

TEMPLATE = app

macx {
	DESTDIR = $$PWD/../../ckb.app/Contents/Resources/ckb-animations
} else {
	DESTDIR = $$PWD/../../bin/ckb-animations
}

SOURCES += \
	main.c
