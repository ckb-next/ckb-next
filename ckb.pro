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
    system(pkg-config --exists libpulse) {
        SUBDIRS += src/ckb-mviz
    }
}
