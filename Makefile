build:
	rm -rf bin
	mkdir bin
	gcc -o bin/ckb-daemon -lusb-1.0 -std=c99 -O2 src/ckb-daemon/main.c
	gcc -o bin/ckb -std=c99 -lm -O2 src/ckb/main.c
