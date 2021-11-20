#include <arm_sve.h>
#include <iostream>
#include <vector>

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

void svshow(svint32_t va){
  int n = svcntw();
  std::vector<int32_t> a(n);
  svbool_t tp = svptrue_b32();
  svst1_s32(tp, a.data(), va);
  for(int i=0;i<n;i++){
    printf("%+03d ", a[n-i-1]);
  }
  printf("\n");
}

int main() {
  int32_t a[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  svint32_t va = svld1_s32(svptrue_b32(), a);
  printf("va = ");
  svshow(va);

  svbool_t tp = svptrue_b32();

  svint32_t vf = svdup_n_s32_x(tp, -1);
  svint32_t vb = svdup_n_s32_x(tp, -2);
  svint32_t vfb = svdup_n_s32_x(tp, -3);

  svint32_t v3= svdup_n_s32_x(tp, 3);
  svint32_t v5= svdup_n_s32_x(tp, 5);
  svint32_t v15= svdup_n_s32_x(tp, 15);

  svint32_t vr = svdiv_s32_z(tp, va, v3);
  vr= svmul_s32_z(tp, vr, v3);
  svbool_t pg= svcmpeq_s32(tp, va, vr);
  svshow(vr);
  show_pr(svptrue_b32());
  show_pr(pg);
  svst1_s32(pg, a, vf);


  for(int i=0;i<16;i++){
    printf("%+03d ", a[i]);
  }
  printf("\n");
}
