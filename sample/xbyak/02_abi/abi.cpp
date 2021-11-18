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
