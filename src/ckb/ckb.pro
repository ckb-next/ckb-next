QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ckb
TEMPLATE = app

QMAKE_MAC_SDK = macosx10.10
ICON = ckb-logo.icns
macx {
	LIBS += -framework AudioToolbox
}

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
	kbprofile.cpp \
	kbanimwidget.cpp \
	animscript.cpp \
    kbanim.cpp \
    animadddialog.cpp

HEADERS  += mainwindow.h \
	kbwidget.h \
	colorbutton.h \
	settingswidget.h \
	kblightwidget.h \
	keymap.h \
	rgbwidget.h \
	media.h \
	kblight.h \
	kbprofile.h \
	kbanimwidget.h \
	animscript.h \
    ckb-anim.h \
    kbanim.h \
    animadddialog.h

FORMS    += mainwindow.ui \
	kbwidget.ui \
	settingswidget.ui \
	kblightwidget.ui \
	kbanimwidget.ui \
    animadddialog.ui

RESOURCES += \
	image.qrc

OBJECTIVE_SOURCES += \
	media_mac.m
