#include <cstdio>
#include <xbyak_aarch64/xbyak_aarch64.h>
#include <vector>

/*
 p0 : all true
 z0 : Data
 z1 : all -1
 z2 : all -2
 z3 : all -3
 z4 : all 3
 z5 : all 5
 z6 : all 15
 */

struct Code : Xbyak_aarch64::CodeGenerator {
  Code() {
    ldr(w1, ptr(x0));
    ptrue(p0.s);
    ld1w(z0.s, p0, ptr(x0));
    dup(z1.s, -1);
    dup(z2.s, -2);
    dup(z3.s, -3);
    dup(z4.s, 3);
    dup(z5.s, 5);
    dup(z6.s, 15);
    //Fizz
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
    cmpeq(p1.s, p0, z0.s, z7.s);
    // Write -2
    st1w(z2.s, p1, ptr(x0));

    // FizzBuzz
    // b[i] = (a[i] / 15) * 15
    mov(z7.s, p0, z0.s);
    sdiv(z7.s, p0.s, z6.s);
    mul(z7.s, p0.s, z6.s);
    // Mask
    cmpeq(p1.s, p0, z0.s, z7.s);
    // Write -3
    st1w(z3.s, p1, ptr(x0));

    ret();
  }
  void dump(const char *filename){
    FILE *fp = fopen(filename, "wb");
    fwrite(getCode(), 1, getSize(), fp);
  }
};

int main() {
  std::vector<int32_t> a(16);
  for(int i=0;i<16;i++){
    a[i] = i + 1;
  }
  Code c;
  auto f = c.getCode<void (*)(int32_t *)>();
  c.ready();
  c.dump("xbyak.dump");
  for(int i=0;i<16;i++){
    printf("%+03d,",a[i]);
  }
  printf("\n");
  f(a.data());
  for(int i=0;i<16;i++){
    printf("%+03d,",a[i]);
  }
  printf("\n");
}
