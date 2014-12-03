#-------------------------------------------------
#
# Project created by QtCreator 2014-11-15T21:35:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ckb
TEMPLATE = app

QMAKE_MAC_SDK = macosx10.10
ICON = ckb-logo.icns
macx {
	LIBS += -framework AudioToolbox
}

OBJECTS_DIR = $$PWD/../../tmp-ckb
DESTDIR = $$PWD/../../bin

SOURCES += main.cpp\
		mainwindow.cpp \
	kbwidget.cpp \
	colorbutton.cpp \
	settingswidget.cpp \
	kblightwidget.cpp \
	keymap.cpp \
	rgbwidget.cpp \
	media_linux.cpp \
	kblight.cpp \
    kbprofile.cpp

HEADERS  += mainwindow.h \
	kbwidget.h \
	colorbutton.h \
	settingswidget.h \
	kblightwidget.h \
	keymap.h \
	rgbwidget.h \
	media.h \
	kblight.h \
    kbprofile.h

FORMS    += mainwindow.ui \
	kbwidget.ui \
	settingswidget.ui \
	kblightwidget.ui

RESOURCES += \
	image.qrc

OBJECTIVE_SOURCES += \
	media_mac.m
