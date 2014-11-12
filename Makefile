DAEMON_SRC := src/ckb-daemon/main.c src/ckb-daemon/usb.c src/ckb-daemon/device.c src/ckb-daemon/input.c src/ckb-daemon/led.c src/ckb-daemon/keyboard.c src/ckb-daemon/devnode.c src/ckb-daemon/notify.c
CKB_SRC := src/ckb/main.c

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	DAEMON_SRC += src/ckb-daemon/usb_linux.c src/ckb-daemon/input_linux.c -ludev -lpthread
endif
ifeq ($(UNAME_S),Darwin)
	DAEMON_SRC += src/ckb-daemon/usb_mac.c src/ckb-daemon/input_mac.c src/ckb-daemon/extra_mac.m -framework IOKit -framework CoreFoundation -framework CoreGraphics -framework AppKit -liconv
endif

build:
	rm -rf bin
	mkdir bin
	gcc $(DAEMON_SRC) -o bin/ckb-daemon -std=c99 -O2 -DKEYMAP_DEFAULT
	gcc $(CKB_SRC) -o bin/ckb -lm -std=c99 -O2 -DKEYMAP_DEFAULT
