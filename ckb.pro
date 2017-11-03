# Major and Minor versions need to be checked separately due to Qt 5.10+
!equals(QT_MAJOR_VERSION, 5) | lessThan(QT_MINOR_VERSION, 2) {
    error("ckb requires at least Qt 5.2!")
}

TEMPLATE = subdirs
CONFIG   += debug_and_release
SUBDIRS = \
    src/ckb-daemon \
    src/ckb \
    src/ckb-ripple \
    src/ckb-wave \
    src/ckb-gradient \
    src/ckb-pinwheel \
    src/ckb-random \
    src/ckb-rain \
    src/ckb-heat

# Music visualizer requires Pulseaudio libraries
linux {
    packagesExist(libpulse libpulse-simple) {
        SUBDIRS += src/ckb-mviz
    }
}
