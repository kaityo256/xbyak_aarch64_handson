#include <cstdio>
#include <xbyak_aarch64/xbyak_aarch64.h>
#include <vector>
#include <random>

struct Code : Xbyak_aarch64::CodeGenerator {
  Code() {
    ldr(w1, ptr(x0));
    ptrue(p0.s);
    ld1w(z0.s, p0, ptr(x0));
    sqadd(z0.s, z0.s, z0.s);
    st1w(z0.s, p0, ptr(x0));
    ret();
  }
};

int main() {
  std::vector<int32_t> a(16), b(16);
  std::mt19937 mt;
  std::uniform_int_distribution ud(1,6);
  for(int i=0;i<16;i++){
    a[i] = ud(mt);
    b[i] = ud(mt);
  }
  Code c;
  auto f = c.getCode<void (*)(int32_t *)>();
  c.ready();
  printf("%d\n",a[0]);
  f(a.data());
  printf("%d\n",a[0]);
}
