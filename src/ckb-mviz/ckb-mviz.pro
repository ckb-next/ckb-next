TEMPLATE = app
TARGET = ckb-mviz

QMAKE_CFLAGS += -std=c99
QMAKE_MAC_SDK = macosx10.10

macx {
    DESTDIR = $$PWD/../../ckb.app/Contents/Resources/ckb-animations
} else {
    DESTDIR = $$PWD/../../bin/ckb-animations
}

CONFIG   =
QT       =
LIBS     = pulse

SOURCES += \
    main.c
