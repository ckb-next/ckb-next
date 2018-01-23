TEMPLATE = app
TARGET = ckb-mviz

QMAKE_CFLAGS += -std=c99


macx {
    DESTDIR = $$PWD/../../ckb.app/Contents/Resources/ckb-animations
} else {
    DESTDIR = $$PWD/../../bin/ckb-animations
}

CONFIG   += debug_and_release
CONFIG   -= app_bundle
QT       =
LIBS += -lpulse-simple

SOURCES += \
    main.c \
	kiss_fft.c \
	kiss_fftr.c
