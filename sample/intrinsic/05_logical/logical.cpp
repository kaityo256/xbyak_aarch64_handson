#include <arm_sve.h>
#include <iostream>
#include <vector>
#include <random>

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

int main() {
  int n = svcntb();
  std::vector<int8_t> a1(n), a2(n);
  std::mt19937 mt(1);
  std::uniform_int_distribution<> ud(0,1);
  for(int i=0;i<n;i++){
    a1[i] = ud(mt);
    a2[i] = ud(mt);
  }
  svbool_t tp = svptrue_b8();
  svint8_t v1= svld1_s8(tp, a1.data());
  svint8_t v2= svld1_s8(tp, a2.data());
  svint8_t vzero= svdup_n_s8_x(tp, 1);
  svbool_t p1 = svcmpeq(tp, v1, vzero);
  svbool_t p2 = svcmpeq(tp, v2, vzero);
  printf("p1        = ");
  show_pr(p1);
  printf("p2        = ");
  show_pr(p2);
  printf("p1 and p2 = ");
  show_pr(svand_b_z(tp, p1, p2));
  printf("\n");
  printf("p1        = ");
  show_pr(p1);
  printf("p2        = ");
  show_pr(p2);
  printf("p1 or p2  = ");
  show_pr(svorr_b_z(tp, p1, p2));
}
