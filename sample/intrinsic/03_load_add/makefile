all: a.out
CC=aarch64-linux-gnu-g++
CPPFLAGS=-static -march=armv8-a+sve -O2

a.out: load_add.cpp
	 $(CC) $(CPPFLAGS) $<

run:
	qemu-aarch64 ./a.out

run128:
	qemu-aarch64 -cpu max,sve128=on ./a.out

run256:
	qemu-aarch64 -cpu max,sve256=on ./a.out

run512:
	qemu-aarch64 -cpu max,sve512=on ./a.out

clean:
	rm -f a.out
