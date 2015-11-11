TEMPLATE = app
TARGET = ckb-wave

QMAKE_CFLAGS += -std=c99
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9

macx {
    DESTDIR = $$PWD/../../ckb.app/Contents/Resources/ckb-animations
} else {
    DESTDIR = $$PWD/../../bin/ckb-animations
}

CONFIG   =
QT       =
LIBS     =

SOURCES += \
    main.c
