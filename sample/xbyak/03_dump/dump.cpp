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

int main() {
  Code c;
  auto f = c.getCode<int (*)(int)>();
  c.ready();
  c.dump("xbyak.dump");
  printf("%d\n", f(10));
}
