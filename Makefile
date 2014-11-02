DAEMON_SRC := src/ckb-daemon/main.c src/ckb-daemon/usb.c src/ckb-daemon/input.c src/ckb-daemon/led.c src/ckb-daemon/keyboard.c src/ckb-daemon/io.c
CKB_SRC := src/ckb/main.c

build:
	rm -rf bin
	mkdir bin
	gcc $(DAEMON_SRC) -o bin/ckb-daemon -lusb-1.0 -std=c99 -O2
	gcc $(CKB_SRC) -o bin/ckb -lm -std=c99 -O2
