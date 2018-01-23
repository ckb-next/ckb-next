TEMPLATE = app
TARGET = ckb-gradient

QMAKE_CFLAGS += -std=c99


macx {
    DESTDIR = $$PWD/../../ckb.app/Contents/Resources/ckb-animations
} else {
    DESTDIR = $$PWD/../../bin/ckb-animations
}

CONFIG   += debug_and_release
CONFIG   -= app_bundle
QT       =
LIBS     =

SOURCES += \
    main.c
