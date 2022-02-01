# Docker Environment for ARM SVE

[TOP500](https://www.top500.org/) is a list of the 500 most powerful computer systems. Currently, the number one computer on this list is a Japanese computer called [Fugaku](https://www.fujitsu.com/global/about/innovation/fugaku/). The architecture of Fugaku is A64fx, which is one of the ARM family. In the field of high performance computing, SIMDs with long vector length are required. The architecture A64fx adopts ARM SVE (Scalable vector Extension). Unlike other SIMD implementations such as AVX2, ARM SVE is scalable, which means that the vector length is not determined at compile time.

While the idea of ARM SVE is interesting, few people can touch an actual machine that implements ARM SVE. So I prepared a Docker environment where you can experience programming with ARM SVE on QEMU.

Here is the tutorial for ARM SVE on Docker.

[kaityo256/xbyak_aarch64_handson](https://github.com/kaityo256/xbyak_aarch64_handson)

## How to obtain a Docker image

Just run the following.

```sh
docker run -it kaityo256/xbyak_aarch64_handson
```

Then you will see something like this.

```sh
[user@2cd82e1ea4e3 ~]$
```

Or, you can build and run the Docker environemnt from Dockerfile as follows.

```sh
git clone https://github.com/kaityo256/xbyak_aarch64_handson.git
cd xbyak_aarch64_handson
cd docker
make
make run
```

## ARM SVE

You can use the ARM SVE instructions in two different ways. The first is to use intrinsic functions. ARM provides intrinsic function of C language, which is called the Arm C Language Extensions (ACLEs). Another way is to use the ARM SVE instructions directly. However, it is hard to write assemblies directly, so I recommend using a JIT assembler called Xbyak. Xbyak is a JIT assembler developed by [MITSUNARI Shigeo](https://github.com/herumi/). Xbyak was initially developed for [x86](https://github.com/herumi/xbyak), but was also released for [AArch64](https://github.com/fujitsu/xbyak_aarch64).

In the following, we will introduce only a simple sample of each. Please see the [tutorial](https://github.com/kaityo256/xbyak_aarch64_handson) for detail.

### ACLEs

The sample codes for the intrinsic functions are in the directory `~/xbyak_aarch64_handson/sample/intrinsic` in the Docker image.

The sample code can be built as follows.

```sh
cd 01_sve_length/
make
```

Then you can run the executable using QEMU.

```sh
$ qemu-aarch64 ./a.out
SVE is available. The length is 512 bits
```

You can specify the vector length in the QEMU options.

```sh
$ qemu-aarch64 -cpu max,sve128=on ./a.out
SVE is available. The length is 128 bits

$ qemu-aarch64 -cpu max,sve256=on ./a.out
SVE is available. The length is 256 bits
```

Here is the source code (`sve_length.cpp`).

```sh
#include <cstdio>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif

int main() {
  int n = 0;
#ifdef __ARM_FEATURE_SVE
  n = svcntb() * 8;
#endif
  if (n) {
    printf("SVE is available. The length is %d bits\n", n);
  } else {
    printf("SVE is unavailable.\n");
  }
}
```

Whether or not ARM SVE can be used can be determined by whether or not __ARM_FEATURE_SVE is defined. If `__ARM_FEATURE_SVE` is defined, then you can use intrinsic functions for SVE by including `arm_sve.h`.

The vector length can be obtained by `svcntb()` which returns the vector length in bytes. The corresponding instruction is `cntb`. The name of a ACLE SVE function consists of a prefix `sv` followed by the corresponding instruction in lower case.

In order to enable SVE, you need to compile with the `-march=armv8-a+sve` option. Without the option, `__ARM_FEATURE_SVE` will not be defined.

```sh
$ aarch64-linux-gnu-g++ -static sve_length.cpp
$ qemu-aarch64 ./a.out
SVE is unavailable.
```

### Xbyak

The sample codes are in the directory `~/xbyak_aarch64_handson/sample/xbyak` in the Docker image. First, let's compile and run the test code.

```sh
$ cd xbyak_aarch64_handson
$ cd sample
$ cd xbyak
$ cd 01_test
$ make
aarch64-linux-gnu-g++ -static test.cpp -L/home/user/xbyak_aarch64_handson/xbyak_aarch64/lib -lxbyak_aarch64
$ ./a.out
1
```

Note that even though `a.out` is a binary for ARM, you can run `a.out` directly without QEMU like this. Even if you don't specify QEMU explicitly, `a.out` is executed through QEMU.

Here is the source code.

```cpp
#include <cstdio>
#include <xbyak_aarch64/xbyak_aarch64.h>

struct Code : Xbyak_aarch64::CodeGenerator {
  Code() {
    mov(w0, 1);
    ret();
  }
};

int main() {
  Code c;
  auto f = c.getCode<int (*)()>();
  c.ready();
  printf("%d\n", f());
}
```

Here, the `mov(w0, 1)` part is where the return value of the function is assigned. Let's change the return value of the function to another value, say 42. Replace the code with `mov(w0, 42)`, and compile and run it again.

```sh
$ make
aarch64-linux-gnu-g++ -static test.cpp -L/home/user/xbyak_aarch64_handson/xbyak_aarch64/lib -lxbyak_aarch64
$ ./a.out
42
```

You will see `42` as the rsult.

## Summary

You can try ARM SVE instructions on QEMU for AArch64. However, preparing that environment is rather tedious, so I prepared a Docker image for it.

For more information about ACLEs and Xbyak_aarch64, please refer to the official documentation.

* [ACLEs (ARM Official Document)](https://developer.arm.com/architectures/system-architectures/software-standards/acle)
* [Xbyak_aarch64 ; JIT assembler for AArch64 CPUs by C++ (GitHub Repository)](https://github.com/fujitsu/xbyak_aarch64).

My English in [the tutorial](https://github.com/kaityo256/xbyak_aarch64_handson) can be poor, so I appreciate pull requests for improvements.

Enjoy ARM SVE!
