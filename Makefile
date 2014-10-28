build:
	rm -rf bin
	mkdir bin
	gcc -o bin/ckb-daemon src/ckb-daemon/main.c -lusb-1.0 -std=c99 -O2
	gcc -o bin/ckb src/ckb/main.c -lm -std=c99 -O2
