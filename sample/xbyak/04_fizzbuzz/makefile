all: a.out

CC=aarch64-linux-gnu-g++
CPPFLAGS=-static
LDFLAGS=-L/home/user/xbyak_aarch64_handson/xbyak_aarch64/lib -lxbyak_aarch64

a.out: fizzbuzz.cpp
	$(CC) $(CPPFLAGS) $< $(LDFLAGS)

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
