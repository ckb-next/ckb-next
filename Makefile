DAEMON_SRC := src/ckb-daemon/main.c src/ckb-daemon/usb.c src/ckb-daemon/input.c src/ckb-daemon/led.c src/ckb-daemon/keyboard.c src/ckb-daemon/devnode.c
CKB_SRC := src/ckb/main.c

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	DAEMON_SRC += src/ckb-daemon/input_linux.c
endif
ifeq ($(UNAME_S),Darwin)
	DAEMON_SRC += src/ckb-daemon/input_mac.c -framework CoreFoundation -framework CoreGraphics -liconv
endif

build:
	rm -rf bin
	mkdir bin
	gcc $(DAEMON_SRC) -o bin/ckb-daemon -I/usr/local/include -L/usr/local/lib -lusb-1.0 -std=c99 -O2
	gcc $(CKB_SRC) -o bin/ckb -lm -std=c99 -O2
