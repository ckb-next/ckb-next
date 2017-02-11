TEMPLATE = app
TARGET = ckb-heat

QMAKE_CFLAGS += -std=c99
QMAKE_MAC_SDK = macosx10.10

macx {
    DESTDIR = $$PWD/../../ckb.app/Contents/Resources/ckb-animations
} else {
    DESTDIR = $$PWD/../../bin/ckb-animations
}

CONFIG   = debug_and_release
QT       =
LIBS     = -lm

SOURCES += \
    main.c
