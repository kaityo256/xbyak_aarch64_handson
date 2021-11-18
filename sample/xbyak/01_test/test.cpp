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
