#include <cstdio>
#include <xbyak_aarch64/xbyak_aarch64.h>

struct Code : Xbyak_aarch64::CodeGenerator {
  Code() {
    mov(w0, 1);
    ret();
  }
};

void dump(const uint8_t *b, int len){
  for(int i=0;i<len;i++){
    printf("%02x",(int)b[i]);
  }
  printf("\n");
}


int main() {
  Code c;
  auto f = c.getCode<int (*)()>();
  c.ready();
  dump(c.getCode(), c.getSize());
}
