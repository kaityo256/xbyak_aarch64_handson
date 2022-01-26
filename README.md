# Hands on: Try Arm SVE on Docker

[Japanese](README_ja.md) / English

## Summary

This is a hands-on document for people who don't have an actual machine to try Arm SVE on Docker.

To have an environment to try Arm SVE with QEMU, just run the following command.

```sh
docker run -it kaityo256/xbyak_aarch64_handson
```

Once the container process is up, you will see a screen like the one below.

```sh
$ docker run -it kaityo256/xbyak_aarch64_handson
[user@2cd82e1ea4e3 ~]$
```

This is an image of ArchLinux with the necessary software pre-installed. In the following, you will try ARM SVE with intrinsic functions and Xbyak_aarch64.

## Contribution

This document is an English translation of the [Japanese version](README_ja.md). The English in this document can be poor, so we appreciate pull requests for improvements.

## Intrinsic Functiosn

You can use Arm SVE instructions via intrinsic function of C language, which is called the Arm C Language Extensions (ACLEs) for SVE. The sample codes for the intrinsic functions are in the directory `~/xbyak_aarch64_handson/sample/intrinsic`.

### 1. SVE Length

Since the length of the vector is *scalble*, and the length is not determined at compile time. So, let's first look at a sample that gets the vector length at runtime.

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

### 2. Predicate registers

Arm SVE adopts *Predicate-centric Approach*. Most of ACLE SVE functions involve predicate registers, which allow you to control whether or not to execute an instruction on an element-by-element basis. The predicate register has different lengths depending on the vector length, and the length is not determined at compile time. Here, we will try to visualize the predicate register.

The type corresponding to the predicate register is `svbool_t`.

The sample code can be built as follows.

```sh
cd 02_predicatemake
make
```

It is useful to prepare a function that takes a variable of type `svbool_t` and prints its bit representation.

```cpp
void show_pr(svbool_t tp) {
  int n = svcntb();
  std::vector<int8_t> a(n);
  std::vector<int8_t> b(n);
  std::fill(a.begin(), a.end(), 1);
  std::fill(b.begin(), b.end(), 0);
  svint8_t va = svld1_s8(tp, a.data());
  svst1_s8(tp, b.data(), va);
  for (int i = 0; i < n; i++) {
    std::cout << (int)b[n - i - 1];
  }
  std::cout << std::endl;
}
```

To set all bits of the predicate register, use `svptrue` function family. For example, to use a predicate register as a byte-by-byte mask, use `svptrue_b8`.

```cpp
show_pr(svptrue_b8());
```

The output will look like this.

```txt
1111111111111111111111111111111111111111111111111111111111111111
```

The function `svptrue_b8()` is equivalent to the function `svptrue_pat_b8` with `SV_ALL` option, and the corresponding assembly is `ptrue p0.b, ALL`.

Similarly, the output results for `svptrue_b16`, `svptrue_b32`, and `svptrue_b64` are as follows.

```txt
svptrue_b16
0101010101010101010101010101010101010101010101010101010101010101
svptrue_b32
0001000100010001000100010001000100010001000100010001000100010001
svptrue_b64
0000000100000001000000010000000100000001000000010000000100000001
```

The correspondence between `svptrue` function family and assembly is as follows.

* `svptrue_b8` => `ptrue p0.b, ALL`
* `svptrue_b16` => `ptrue p0.h, ALL`
* `svptrue_b32` => `ptrue p0.s, ALL`
* `svptrue_b64` => `ptrue p0.d, ALL`

There are various ways to give patterns to the predicate registers, for example, `SV_VL1` means "set a bit from LSB", and `VL2` means "set two bits up two from LSB". Let's see how it works.

```cpp
void ptrue_pat() {
  std::cout << "# pture_pat samples for vrious patterns" << std::endl;
  std::cout << "svptrue_pat_b8(SV_ALL)" << std::endl;
  show_pr(svptrue_pat_b8(SV_ALL));
  std::cout << "svptrue_pat_b8(SV_VL1)" << std::endl;
  show_pr(svptrue_pat_b8(SV_VL1));
  std::cout << "svptrue_pat_b8(SV_VL2)" << std::endl;
  show_pr(svptrue_pat_b8(SV_VL2));
  std::cout << "svptrue_pat_b8(SV_VL3)" << std::endl;
  show_pr(svptrue_pat_b8(SV_VL3));
  std::cout << "svptrue_pat_b8(SV_VL4)" << std::endl;
  show_pr(svptrue_pat_b8(SV_VL4));
}
```

The output will be as follows.

```txt
# pture_pat samples for vrious patterns
svptrue_pat_b8(SV_ALL)
1111111111111111111111111111111111111111111111111111111111111111
svptrue_pat_b8(SV_VL1)
0000000000000000000000000000000000000000000000000000000000000001
svptrue_pat_b8(SV_VL2)
0000000000000000000000000000000000000000000000000000000000000011
svptrue_pat_b8(SV_VL3)
0000000000000000000000000000000000000000000000000000000000000111
svptrue_pat_b8(SV_VL4)
0000000000000000000000000000000000000000000000000000000000001111
```

The position of the bits to be set depend on the type. Let's see which bits are set by `SV_VL2` for various types.

Here is the source code.

```cpp
void ptrue_pat_types() {
  std::cout << "# pture_pat samples for various types" << std::endl;
  std::cout << "svptrue_pat_b8(SV_VL2)" << std::endl;
  show_pr(svptrue_pat_b8(SV_VL2));
  std::cout << "svptrue_pat_b16(SV_VL2)" << std::endl;
  show_pr(svptrue_pat_b16(SV_VL2));
  std::cout << "svptrue_pat_b32(SV_VL2)" << std::endl;
  show_pr(svptrue_pat_b32(SV_VL2));
  std::cout << "svptrue_pat_b64(SV_VL2)" << std::endl;
  show_pr(svptrue_pat_b64(SV_VL2));
}
```

And here are the outputs.

```txt
# pture_pat samples for various types
svptrue_pat_b8(SV_VL2)
0000000000000000000000000000000000000000000000000000000000000011
svptrue_pat_b16(SV_VL2)
0000000000000000000000000000000000000000000000000000000000000101
svptrue_pat_b32(SV_VL2)
0000000000000000000000000000000000000000000000000000000000010001
svptrue_pat_b64(SV_VL2)
0000000000000000000000000000000000000000000000000000000100000001
```

You can change the vector length and see how the results change.

```sh
qemu-aarch64 -cpu max,sve128=on ./a.out
qemu-aarch64 -cpu max,sve256=on ./a.out
qemu-aarch64 -cpu max,sve512=on ./a.out
```

### 3. Vector operations

In order to use SIMD instructions, data should be loaded into the SIMD registers. In the following, we will see how the loading of the registers and the arithmetic operations are performed, and how mask processing can be performed using the predicate register.

The sample code is in `sample/intrinsic/03_load_add`.

The name of the SVE ACLE type corresponds to the type of the element contained in the SIMD register. For example, the variable that stores `float64_t` is `svfloat64_t`, and since SVE registers are not specified in length, it is not known at compile time how many `float64_t` variables are stored in the register.

It is useful to prepare a function to display the contents of the SIMD register.

```cpp
void svshow(svfloat64_t va){
  int n = svcntd();
  std::vector<double> a(n);
  svbool_t tp = svptrue_b64();
  svst1_f64(tp, a.data(), va);
  for(int i=0;i<n;i++){
    printf("%+.7f ", a[n-i-1]);
  }
  printf("\n");
}
```

Use `svld1_f64` to load into `svfloat64_t`. If you pass the predicate register and the first address, it will fetch the data as wide as the register. The code to define an array, load it into a register from it, and display the register can be written as follows.

```cpp
  double a[] = {0, 1, 2, 3, 4, 5, 6, 7};
  svfloat64_t va = svld1_f64(svptrue_b64(), a);
  printf("va = ");
  svshow(va);
```

Here is the result.

```txt
va = +7.0000000 +6.0000000 +5.0000000 +4.0000000 +3.0000000 +2.0000000 +1.0000000 +0.0000000
```

Similarly, a register with all elements set to 1 is also prepared.

```cpp
  double b[] = {1, 1, 1, 1, 1, 1, 1, 1};
  svfloat64_t vb = svld1_f64(svptrue_b64(), b);
  printf("vb = ");
  svshow(vb);
  printf("\n");
```

Use `svadd_f64_z` to add `svfloat64_t`.

```cpp
  svfloat64_t vc1 = svadd_f64_z(svptrue_b64(), va, vb);
  printf("va + vb = ");
  svshow(vc1);
```

Here is the result.

```txt
va + vb = +8.0000000 +7.0000000 +6.0000000 +5.0000000 +4.0000000 +3.0000000 +2.0000000 +1.0000000
```

By specifying a pattern in the predicate register, it is possible to mask where the addition is to be performed. For example, if `SV_VL2` is specified, only two operations are performed from the lower address.

For operations where the predicate register is inactive, you can choose to clear zero or merge the first argument.Using `svadd_f64_z`, inactive elements will be cleared to zero as follows (zeroing predication).

```cpp
  svfloat64_t vc2 = svadd_f64_z(svptrue_pat_b64(SV_VL2), va, vb);
  printf("va + vb = ");
  svshow(vc2);
```

```txt
va + vb = +0.0000000 +0.0000000 +0.0000000 +0.0000000 +0.0000000 +0.0000000 +2.0000000 +1.0000000
```

Using `svadd_f64_m`, which changes the last z of `svadd_f64_z` to m, inactive elements will merge the first argument.

```cpp
  svfloat64_t vc3 = svadd_f64_m(svptrue_pat_b64(SV_VL2), va, vb);
  printf("va + vb = ");
  svshow(vc3);
```

```txt
va + vb = +7.0000000 +6.0000000 +5.0000000 +4.0000000 +3.0000000 +2.0000000 +2.0000000 +1.0000000
```

In SVE, you need to code for variable length SIMD registers like this, making full use of the masking process.

### 4. Fizz Buzz Implementation with ACLE SVE

Let's try to write FizzBuzz as an example of code that makes full use of scalable SIMD registers and mask operations. Instead of displaying as Fizz or Buzz, we replace elements with -1, -2, or -3 when they are multiple of 3, 5, or 15, respectively.

The serial code looks like this.

```cpp
#include <cstdio>
#include <vector>

int main() {
  // init
  const int n = 32;
  std::vector<int32_t> a(n);
  for (int i = 0; i < n; i++) {
    a[i] = i + 1;
  }
  // FizzBuzz
  for (int i = 0; i < n; i++) {
    if (a[i] % 15 == 0) {
      a[i] = -3;
    } else if (a[i] % 3 == 0) {
      a[i] = -1;
    } else if (a[i] % 5 == 0) {
      a[i] = -2;
    }
  }
  // Show Results
  for (int i = 0; i < n; i++) {
    if (a[i] == -1) {
      puts("Fizz");
    } else if (a[i] == -2) {
      puts("Buzz");
    } else if (a[i] == -3) {
      puts("FizzBuzz");
    } else {
      printf("%d\n", a[i]);
    }
  }
}
```

Now, this code consists of three parts: initialization, FizzBuzz, and result display. We rewrite the FizzBuzz part with SVE.

First of all, let's deal with the if statement. We use mask operations to express if statements. To make the mask, we fist divide `a[i]` by 3, then multiply it by 3, and determine whether it is a multiple of 3 or not by matching the original number.

It would look like this.

```cpp
  // FizzBuzz
  for (int i = 0; i < n; i++) {
    uint32_t t = a[i];
    uint32_t r3 = (t / 3) * 3;
    if (r3 == t) {
      a[i] = -1;
    }
    uint32_t r5 = (t / 5) * 5;
    if (r5 == t) {
      a[i] = -2;
    }
    uint32_t r15 = (t / 15) * 15;
    if (r15 == t) {
      a[i] = -3;
    }
  }
```

We will rewrite this part with intrinsic functions. We don't know the vector length of SVE until runtime, but for simplicity, let's assume that `n` is always a multiple of the number of elements corresponding to the vector length.

Before entering the loop, we prepare registers that contain the necessary constants. ARM SVE intrinsic functions will always receive a predicator register, so we prepare a register which is all true.

```cpp
  svbool_t tp = svptrue_b32();
```

Next, we make a vector filled with -1, -2, and -3 for value assignment. `vf`, `vb`, and `vfb` denote "Vector for Fizz", "Vector for Buzz", and "Vector for FizzBuzz", respectively.

```cpp
  svint32_t vf = svdup_n_s32_x(tp, -1);
  svint32_t vb = svdup_n_s32_x(tp, -2);
  svint32_t vfb = svdup_n_s32_x(tp, -3);
```

We also make a vector filled with 3, 5, and 15 for divisions and multiplications.

```cpp
  svint32_t v3 = svdup_n_s32_x(tp, 3);
  svint32_t v5 = svdup_n_s32_x(tp, 5);
  svint32_t v15 = svdup_n_s32_x(tp, 15);
```

How many integers, i.e. `uint32_t`, can be stored in SVE registers can be obtained with `cntw`. The corresponding intrinsic function is `svcntw`. Therefore, the loop structure looks as follows.

```cpp
  int w = svcntw();
  int s = 0;
  while (s + w <= n) {
      // FizzBuzz
    s += w;
  }
```

Here, s is the index of the beginning of the data to be operated.

To load `w` data from the `s`th index of `std::vector<int32_t> a(n)` into a SIMD register, we use `svld1_s32`.

```cpp
svint32_t va = svld1_s32(svptrue_b32(), a.data() + s);
```

We prepare a temporaly variable `svint32_t vr` to store the value of `va` divided by 3. The function for integer division is `svdiv_s32_z`.

```cpp
svint32_t vr;
vr = svdiv_s32_z(tp, va, v3);
```

We next multiply by 3. The mfunction for ultiplication of integers is `svmul_s32_z`.

```cpp
vr = svmul_s32_z(tp, vr, v3);
```

Now `vr` stores the values of `va` divided by 3 and multiplied by 3. Compare each element between `va` and `vr`, and the location that matches is a multiple of 3. We put the matching locations into the predicate register by `svcmpeq_s32`, which compares the two vector registers as if they were `uint32_t`, and returns the predicate register with the matching location.

```cpp
svbool_t pg;
pg = svcmpeq_s32(tp, va, vr);
```

Since the positions of multiples of 3 are now stored in `pg`, we use it to write the register `vf` filled with -1 back to `a`.

```cpp
svst1_s32(pg, a.data() + s, vf);
```

The same goes for multiples of 5 and multiples of 15. Putting all the above together, the code looks like the following.

```cpp
  // FizzBuzz
  svbool_t tp = svptrue_b32();
  svint32_t vf = svdup_n_s32_x(tp, -1);
  svint32_t vb = svdup_n_s32_x(tp, -2);
  svint32_t vfb = svdup_n_s32_x(tp, -3);

  svint32_t v3 = svdup_n_s32_x(tp, 3);
  svint32_t v5 = svdup_n_s32_x(tp, 5);
  svint32_t v15 = svdup_n_s32_x(tp, 15);

  int w = svcntw();
  int s = 0;

  while (s + w <= n) {
    svint32_t va = svld1_s32(svptrue_b32(), a.data() + s);

    svint32_t vr;
    svbool_t pg;
    vr = svdiv_s32_z(tp, va, v3);
    vr = svmul_s32_z(tp, vr, v3);
    pg = svcmpeq_s32(tp, va, vr);
    svst1_s32(pg, a.data() + s, vf);

    vr = svdiv_s32_z(tp, va, v5);
    vr = svmul_s32_z(tp, vr, v5);
    pg = svcmpeq_s32(tp, va, vr);
    svst1_s32(pg, a.data() + s, vb);

    vr = svdiv_s32_z(tp, va, v15);
    vr = svmul_s32_z(tp, vr, v15);
    pg = svcmpeq_s32(tp, va, vr);
    svst1_s32(pg, a.data() + s, vfb);
    s += w;
  }
```

The sample code is in `sample/intrinsic/04_fizzbuzz`. You can build and run the sample as follows.

```sh
$ make
aarch64-linux-gnu-g++ -static -march=armv8-a+sve -O2 fizzbuzz.cpp
$ ./a.out
1
2
Fizz
4
Buzz
(snip)
26
Fizz
28
29
FizzBuzz
31
32
```

You can confirm that changing the register length does not change the result.

```sh
qemu-aarch64 -cpu max,sve128=on ./a.out
qemu-aarch64 -cpu max,sve256=on ./a.out
qemu-aarch64 -cpu max,sve512=on ./a.out
```

You should see the same results for all of the aboves.

## Xbyak_aarch64

### 1. Test

First of all, let's test the operation of Xbyak_aarch64. The sample codes are in `sample/xbyak`. First, let's compile and run the test code.

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

### 2. Calling convention

Xbyak is a tool for writing a function in full assembly. In assembly, function calls are jumps, and registers and other variables are all global variables, so it is up to the programmer to decide how to pass function arguments and how to return values. However, when using a high-level language such as C, it is inconvenient if each compiler has different calling conventions, because object files compiled by different compilers cannot be linked. The Application Binary Interface (ABI) defines binary-level interfaces for each ISA, and calling conventions are one of the many things defined by the ABI. In the following, we will take a brief look at the calling convention and how to write Xbyak.

In the directory `/sample/xbyak/02_abi`, there is `abi.cpp` as a template code of Xbyak.

```cpp
#include <cstdio>
#include <xbyak_aarch64/xbyak_aarch64.h>

struct Code : Xbyak_aarch64::CodeGenerator {
  Code() {
    ret();
  }
};

int main() {
  Code c;
  auto f = c.getCode<void (*)()>();
  c.ready();
}
```

This code is a sample that creates and executes a function that does nothing but simply return when called. Since the signature of the function is `void f()`, we pass the type `void (*)()` to getCode. First, we will modify it to a function that returns an integer 1. To do this, we need to know how to return an integer in AAarch64. Of course, you can read [the official document](https://github.com/fujitsu/A64FX) (136 pages, wow!), but it's not practical to read or memorize it every time. Here it is easy to write a simple code and compile it. So, let's write some simple code and compile it to see the calling convention.

Consider a code like the following.

```cpp
int func(){
  return 1;
}
```

Compile it and see the assembly.

```sh
$ ag++ -S test.cpp
$ cat test.s
        .arch armv8-a+sve
        .file   "test.cpp"
        .text
        .align  2
        .p2align 4,,11
        .global _Z4funcv
        .type   _Z4funcv, %function
_Z4funcv:
.LFB0:
        .cfi_startproc
        mov     w0, 1
        ret
        .cfi_endproc
.LFE0:
        .size   _Z4funcv, .-_Z4funcv
        .ident  "GCC: (GNU) 11.2.0"
        .section        .note.GNU-stack,"",@progbits
```

This shows that an integer should be returned by putting their value in the register `w0`. From here, we can see that we should modify `abi.cpp` as follows.

```cpp
#include <cstdio>
#include <xbyak_aarch64/xbyak_aarch64.h>

struct Code : Xbyak_aarch64::CodeGenerator {
  Code() {
    mov(w0, 1); // mov w0, 1 
    ret();
  }
};

int main() {
  Code c;
  auto f = c.getCode<int (*)()>(); // Fixed function pointer type to correspond to int f().
  c.ready();
  printf("%d\n",f()); // Display the result of f()
}
```

Let's compile and run it.

```sh
$ make
$ ./a.out
1
```

You will see 1 as the result.

Next, let's take arguments. Let's consider a function that takes an integer and returns a value added by 1. As before, let's ask the compiler to tell us the assembly.

```cpp
int func(int i){
  return i+1;
}
```

If you compile the above code with `ag++ -S`, you will see that the corresponding assembly looks like the following.

```txt
  add w0, w0, 1
  ret
```

That is, the first integer argument comes in `w0`, so we should assign the result of adding `1` to it to `w0`. The corresponding code of Xbyak will be as follows.

```cpp
#include <cstdio>
#include <xbyak_aarch64/xbyak_aarch64.h>

struct Code : Xbyak_aarch64::CodeGenerator {
  Code() {
    add(w0, w0, 1);
    ret();
  }
};

int main() {
  Code c;
  auto f = c.getCode<int (*)(int)>(); // the signature of function is changed to be `int f(int)`
  printf("%d\n",f(1)); // call f(1)
  c.ready();
}
```

You will see 2 as the results.

```sh
$ make
$ ./a.out
2
```

In similar manner, we can write a function that takes two arguments and returns the sum as follows.

```cpp
#include <cstdio>
#include <xbyak_aarch64/xbyak_aarch64.h>

struct Code : Xbyak_aarch64::CodeGenerator {
  Code() {
    add(w0, w0, w1); // w0 = w0 + w1
    ret();
  }
};

int main() {
  Code c;
  auto f = c.getCode<int (*)(int, int)>(); // the signature is changed to be `int f(int, int)`
  printf("%d\n",f(3,4)); // calculate 3+4
  c.ready();
}
```

You will get 7 as a result of adding 3 to 4.

Since we treat integer operation here, he registers were `w0, w1` and the add instruction was `add`. If we change the registers to `d0, d1` and the add instruction to `fadd`, we can make it a double-precision version.

```cpp
#include <cstdio>
#include <xbyak_aarch64/xbyak_aarch64.h>

struct Code : Xbyak_aarch64::CodeGenerator {
  Code() {
    fadd(d0, d0, d1); // d0 = d0 + d1
    ret();
  }
};

int main() {
  Code c;
  auto f = c.getCode<double (*)(double, double)>(); // double f(double, double);
  printf("%f\n",f(3.0,4.0)); // calculate 3.0+4.0
  c.ready();
}
```

You will see `7.000000` as the result.

### 3. Display the assembler mnemonics generated by Xbyak

Xbyak is a tool that puts assembly instructions in memory and executes them from the first address. What kind of assembly instructions are placed depends on the program you have created. Therefore, we are writing a C / C ++ program that outputs assembly, that is, a code generator.

When there is a bug in the code generator, we want to debug it by looking at the output assembly. Therefore, we will see how to disassemble the code generated by Xbyak. The sample code is `sample/xbyak/03_dump/dump.cpp`.

To get a machine language generated by Xbyak, use `Xbyak_aarch64::CodeGenerator::getCode()`. You can also get the length of a machine language with `getSize()`. Let's create a method to save it with a name. Add a method `dump` for saving the machine language to the Xbyak as follows.

```cpp
#include <cstdio>
#include <xbyak_aarch64/xbyak_aarch64.h>

struct Code : Xbyak_aarch64::CodeGenerator {
  Code() {
    mov(w0, 1);
    ret();
  }
  void dump(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    fwrite(getCode(), 1, getSize(), fp);
  }
};
```

`void dump(const char *filename)` is the code to save the machine language created by Xbyak with a name.

The code that executes the code generated by Xbyak, but also saves its machine language, can be written as follows.

```cpp
int main() {
  Code c;
  auto f = c.getCode<int (*)(int)>();
  c.ready();
  c.dump("xbyak.dump");
  printf("%d\n",f(10));
}
```

Here is the results.

```sh
$ make
$ ./a.out
1
```

The first `1` is the result of the function generated by Xbyak. The machine language generated by Xbyak is saved as `xbyak.dump`. You can disassemble it by passing it to `objdump`, but you need to give it some information because it has no header information.

```sh
$ aarch64-linux-gnu-objdump -D -maarch64 -b binary -d xbyak.dump

xbyak.dump:     file format binary


Disassembly of section .data:

0000000000000000 <.data>:
   0:   52800020        mov     w0, #0x1                        // #1
   4:   d65f03c0        ret
```

You can see that the code that `mov w0, 1; ret` is generated as intended.

Since it is troublesome to type `aarch64-linux-gnu-objdump -D -maarch64 -b binary -d` every time, the following alias is defined in `.bashrc`.

```sh
alias xdump="aarch64-linux-gnu-objdump -D -maarch64 -b binary -d"
```

You can use it as follows.

```sh
xdump xbyak.dump
```

Now, let's see how Xbyak generates code dynamically. Modify the function so that adds 1 to n times and returns it.

```cpp
struct Code : Xbyak_aarch64::CodeGenerator {
  Code(int n) {
    for(int i=0;i<n;i++){
      add(w0, w0, 1);
    }
    ret();
  }
};
```

The constructor `Code` recieves `int n` and repeats `add(w0, w0, 1);` as many times as it takes. Specify the number of iterations as `Code c(3);`.

```cpp
int main() {
  Code c(3); // Modified here
  auto f = c.getCode<int (*)(int)>();
  c.ready();
  printf("%d\n", f(10));
  dump(c.getCode(), c.getSize());
}
```

You can build and run as follows.

```sh
$ make
$ ./a.out
13
```

As a result of execution, 13 which is the number of 10 plus 1 three times was displayed. Here are the assembly codes.

```sh
$ xdump xbyak.dump

xbyak.dump:     file format binary


Disassembly of section .data:

0000000000000000 <.data>:
   0:   11000400        add     w0, w0, #0x1
   4:   11000400        add     w0, w0, #0x1
   8:   11000400        add     w0, w0, #0x1
   c:   d65f03c0        ret
```

As intended, the code execute `add` three times. This is generated at runtime, so it doesn't have to be fixed at compile time. Let's feed it from the standard input.

```cpp
int main(int argc, char **argv) {
  Code c(atoi(argv[1]));
  auto f = c.getCode<int (*)(int)>();
  c.ready();
  printf("%d\n", f(10));
  dump(c.getCode(), c.getSize());
}
```

You can feed any number, say 5.

```sh
$ ./a.out  5
15

$ xdump xbyak.dump

xbyak.dump:     file format binary


Disassembly of section .data:

0000000000000000 <.data>:
   0:   11000400        add     w0, w0, #0x1
   4:   11000400        add     w0, w0, #0x1
   8:   11000400        add     w0, w0, #0x1
   c:   11000400        add     w0, w0, #0x1
  10:   11000400        add     w0, w0, #0x1
  14:   d65f03c0        ret
```

You can see that Xbyak generates code dynamically.

### 4. Fizz Buzz Implementation with Xbyak

Finally, let's try to write FizzBuzz with Xbyak. As in the example of instrinsic functions, FizzBuzz is expressed by writing -1 for multiples of 3, -2 for multiples of 5, and -3 for multiples of 15 to an array of integers of type `int32_t`. The data are stored in `std::vector` of the type `int32_t`, and the first address is passed as an argument of the function made by Xbyak.

The algorithm is as follows.

* Set all bits of `p0` to true.
* Prepare registers with all elements initialized with `-1`, `-2`, `-3`, `3`, and `5` (in order, `z1` to `z5`).
* Load the values of the array to the register `z0`. Since `int32_t` is used, 16 elements can be loaded at a time (when the SVE register is 512-bit-width).
* Copy the `z0` register to the `z7` register.
* Divide all elements in the `z7` register by 3, then multiply by 3 (use `sdiv` and `mul`).
* Compare the `z7` register with the `z0` register, make a mask with the bits of the matching places up and put it in `p1`.
* Write the `z1` register back to the address of the array with the `p1` register as a mask (Fizz).
* Buzz is calculated in the same way, and the mask information is put into `p2` and written back.
* For FizzBuzz, put the mask information `p1` and `p2` logical conjunction (AND) into `p3`, and write back `-3` using the mask.

Although the code is not very efficient, it is good for practicing SVE with Xbyak because it contains processing multiple elements at once using SVE, mask store using predicate registers, and logical operations with predicate registers.

The source code is in `fizzbuzz.cpp` in the directory `/sample/xbyak/04_fizzbuzz`.

First, make the code of Xbyak to get how many variables of type `int32_t` are in the SVE register.

```cpp
struct Cntw : Xbyak_aarch64::CodeGenerator {
  Cntw() {
    cntw(x0);
    ret();
  }
};
```

It simply calls `cntw` and puts it in `x0`. You can get the number of elements as follows.

```cpp
  Cntw cw;
  int nw = cw.getCode<int (*)()>()();
  printf("Number of int32_t in a register is %d.\n",nw);
```

The signature of the FizzBuzz function generated by Xbyak is as follows.

```cpp
void f(int32_t *);
```

Therefore, the type passed to getCode is as follows.

```cpp
auto f = c.getCode<void (*)(int32_t *)>();
```

The generated function can be called as follows. `a` is a variable of type `std::vector<int32_t>`.

```cpp
f(a.data());
```

Positive integers are stored in the vector `a`, and when `f` is called, multiples of 3 are rewritten to -1, multiples of 5 to -2, and multiples of 15 to -3, respectively. The code of Xbyak to make such a function `f` is as follows.

```cpp
  Code(int n, int nw) {
    ptrue(p0.s);
    dup(z1.s, -1);
    dup(z2.s, -2);
    dup(z3.s, -3);
    dup(z4.s, 3);
    dup(z5.s, 5);
    for (int i = 0; i < n/nw; i++) {
      ld1w(z0.s, p0, ptr(x0));
      // Fizz
      // b[i] = (a[i] / 3) * 3
      mov(z7.s, p0, z0.s);
      sdiv(z7.s, p0.s, z4.s);
      mul(z7.s, p0.s, z4.s);
      // Mask
      cmpeq(p1.s, p0, z0.s, z7.s);
      // Write -1
      st1w(z1.s, p1, ptr(x0));

      // Buzz
      // b[i] = (a[i] / 5) * 5
      mov(z7.s, p0, z0.s);
      sdiv(z7.s, p0.s, z5.s);
      mul(z7.s, p0.s, z5.s);
      // Mask
      cmpeq(p2.s, p0, z0.s, z7.s);
      // Write -2
      st1w(z2.s, p2, ptr(x0));

      // FizzBuzz
      and_(p3.b, p0, p1.b, p2.b);
      // Write -3
      st1w(z3.s, p3, ptr(x0));

      adds(x0, x0, nw*4);
    }

    ret();
  }
  void dump(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    fwrite(getCode(), 1, getSize(), fp);
  }
};
```

Let me explain step by step.

First, it takes the number of elements `n` and the number of elements per register `nw` in the constructor, and generates the code that expands the loop by that amount. Note that these arguments are for the code generator, not for the function generated by Xbyak.

We prepare the registers that store the constants.

```cpp
ptrue(p0.s);    // all true
dup(z1.s, -1);  // filled with -1
dup(z2.s, -2);  // filled with -2
dup(z3.s, -3);  // filled with -3
dup(z4.s, 3);   // filled with 3
dup(z5.s, 5);   // filled with 5
```

The function `f` is passed `f(a.data())` and the first address of `a`. This address is in `x0`, and we use it to load data into the `z0` register with the `ld1w` instruction. At this time, since we bring all the data, we use `p0`, where all the data is true.

```cpp
ld1w(z0.s, p0, ptr(x0));
```

The elements of `a` are now stored together in `z0`.

Next, copy the value of `z0` to `z7`.

```cpp
mov(z7.s, p0, z0.s);
```

Divide all the elements of `z7` by 3, and then multiply by 3. We have a register `z4` whose elements are all 3, so we can use it.

```cpp
sdiv(z7.s, p0.s, z4.s);
mul(z7.s, p0.s, z4.s);
```

We divide the elements by 3 (rounded down to the nearest whole number) and multiply by 3. So for example, if the register `z0` stores "1,2,3,4", then values in `z7` will be "0,0,3,3". We make a mask by comparing these `z0` and `z7` with `cmpeq.

```cpp
cmpeq(p1.s, p0, z0.s, z7.s);
```

Then the predicate register `p1` will be `0010`. We can use it as a mask to write the data "-1,-1,-1,-1" to the address of array `a`. Use the register `z1` with all elements initialized to -1.

```cpp
st1w(z1.s, p1, ptr(x0));
```

In this way, the data that was originally "1,2,3,4" became "1,2, -1,4". You can handle Buzz in exactly the same way.

Next, we will make a mask for FizzBuzz, i.e., a place divisible by 15. Since we already have the predicate register `p1`, which stores the positions for divisible by 3 and the register `p2` for divisible by 5, we can make a mask that stores the positions for divisible by 15 `p3` by performing logical conjunction between `p1` and `p2`.

```cpp
and_(p3.b, p0, p1.b, p2.b);
```

Since `and` is a reserved word, Xbyak seems to name it `and_`. Then, you can write back `-3` in the same way.

Finally, shift the value of register `x0`, which is the address to be read or written, by the number of elements per register * 4 bytes (= `sizeof(int32_t)`).

```cpp
adds(x0, x0, nw*4);
```

Repeat this as many times as necessary to complete the process for all the elements. While we can create a loop with Xbyak, let's unroll the loop completely to take advantage of the JIT.

You will obtain the following results by compiling and executing the code.

```sh
$ make
$ ./a.out
Number of int32_t in a register is 16.
1
2
Fizz
4
Buzz
Fizz
7
8
Fizz
Buzz
11
Fizz
13
14
FizzBuzz
16
17
Fizz
19
Buzz
Fizz
22
23
Fizz
Buzz
26
Fizz
28
29
FizzBuzz
31
32
```

Let's also take a look at the machine language generated by Xbyak.

```sh
$ xdump xbyak.dump

xbyak.dump:     file format binary


Disassembly of section .data:

0000000000000000 <.data>:
   0:   2598e3e0        ptrue   p0.s
   4:   25b8dfe1        mov     z1.s, #-1
   8:   25b8dfc2        mov     z2.s, #-2
   c:   25b8dfa3        mov     z3.s, #-3
  10:   25b8c064        mov     z4.s, #3
  14:   25b8c0a5        mov     z5.s, #5
  18:   a540a000        ld1w    {z0.s}, p0/z, [x0]
  1c:   05a7c007        mov     z7.s, p0/m, z0.s
  20:   04940087        sdiv    z7.s, p0/m, z7.s, z4.s
  24:   04900087        mul     z7.s, p0/m, z7.s, z4.s
  28:   2487a001        cmpeq   p1.s, p0/z, z0.s, z7.s
  2c:   e540e401        st1w    {z1.s}, p1, [x0]
  30:   05a7c007        mov     z7.s, p0/m, z0.s
  34:   049400a7        sdiv    z7.s, p0/m, z7.s, z5.s
  38:   049000a7        mul     z7.s, p0/m, z7.s, z5.s
  3c:   2487a002        cmpeq   p2.s, p0/z, z0.s, z7.s
  40:   e540e802        st1w    {z2.s}, p2, [x0]
  44:   25024023        and     p3.b, p0/z, p1.b, p2.b
  48:   e540ec03        st1w    {z3.s}, p3, [x0]
  4c:   b1010000        adds    x0, x0, #0x40
  50:   a540a000        ld1w    {z0.s}, p0/z, [x0]
  54:   05a7c007        mov     z7.s, p0/m, z0.s
  58:   04940087        sdiv    z7.s, p0/m, z7.s, z4.s
  5c:   04900087        mul     z7.s, p0/m, z7.s, z4.s
  60:   2487a001        cmpeq   p1.s, p0/z, z0.s, z7.s
  64:   e540e401        st1w    {z1.s}, p1, [x0]
  68:   05a7c007        mov     z7.s, p0/m, z0.s
  6c:   049400a7        sdiv    z7.s, p0/m, z7.s, z5.s
  70:   049000a7        mul     z7.s, p0/m, z7.s, z5.s
  74:   2487a002        cmpeq   p2.s, p0/z, z0.s, z7.s
  78:   e540e802        st1w    {z2.s}, p2, [x0]
  7c:   25024023        and     p3.b, p0/z, p1.b, p2.b
  80:   e540ec03        st1w    {z3.s}, p3, [x0]
  84:   b1010000        adds    x0, x0, #0x40
  88:   d65f03c0        ret
```

You can see that the code created by Xbyak has been doubly expanded.

The current result is for 512-bit registers, but you can also try it for 128-bit and 256-bit registers.

```sh
$ make run128
qemu-aarch64 -cpu max,sve128=on ./a.out
Number of int32_t in a register is 4.
1
2
Fizz
(snip)

$ make run256
qemu-aarch64 -cpu max,sve256=on ./a.out
Number of int32_t in a register is 8.
1
2
Fizz
(snip)
```

If you disassemble `xbyak.dump` after each execution, you can see that the loops are expanded 8 times and 4 times, it shows that the scalable codes are generated dynamically.

## Licence

MIT
