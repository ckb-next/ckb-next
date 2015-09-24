TEMPLATE = subdirs
#CONFIG   += debug_and_release
CONFIG = debug
SUBDIRS = \
	src/ckb-daemon \
	src/ckb \
	src/ckb-ripple \
	src/ckb-wave \
	src/ckb-gradient \
	src/ckb-pinwheel \
	src/ckb-random \
	src/ckb-rain \
	src/ckb-heat \
	src/ckb-mviz

QMAKE_MAC_SDK = macosx10.10
