all: a.out

CC=aarch64-linux-gnu-g++
CPPFLAGS=-static
LDFLAGS=-L/home/user/xbyak_aarch64_handson/xbyak_aarch64/lib -lxbyak_aarch64

a.out: abi.cpp
	$(CC) $(CPPFLAGS) $< $(LDFLAGS)


clean:
	rm -f a.out
