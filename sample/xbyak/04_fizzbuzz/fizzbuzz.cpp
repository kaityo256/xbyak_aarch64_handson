#include <cstdio>
#include <vector>
#include <xbyak_aarch64/xbyak_aarch64.h>

/*
 p0 : all true
 p1 : mask for Fizz
 p2 : mask for Buzz
 p3 : mask for FizzBuzz
 z0 : Data
 z1 : all -1
 z2 : all -2
 z3 : all -3
 z4 : all 3
 z5 : all 5
 */

struct Code : Xbyak_aarch64::CodeGenerator {
  Code(int n) {
    ptrue(p0.s);
    dup(z1.s, -1);
    dup(z2.s, -2);
    dup(z3.s, -3);
    dup(z4.s, 3);
    dup(z5.s, 5);
    for (int i = 0; i < n / 16; i++) {
      adds(x0, x0, i * 64);
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
    }

    ret();
  }
  void dump(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    fwrite(getCode(), 1, getSize(), fp);
  }
};

int main() {
  int n = 32;
  std::vector<int32_t> a(n);
  for (int i = 0; i < n; i++) {
    a[i] = i + 1;
  }
  Code c(n);
  auto f = c.getCode<void (*)(int32_t *)>();
  c.ready();
  c.dump("xbyak.dump");
  f(a.data());

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
