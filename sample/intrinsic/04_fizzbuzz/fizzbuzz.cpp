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

const int n = 64;

int main() {
  std::vector<int32_t> a(n);
  for(int i=0;i<n;i++){
    a[i] = i+1;
  }

  int w = svcntw();
  int s = 0;
  while (s + w <= n){
    svint32_t va = svld1_s32(svptrue_b32(), a.data()+s);

    svbool_t tp = svptrue_b32();

    svint32_t vf = svdup_n_s32_x(tp, -1);
    svint32_t vb = svdup_n_s32_x(tp, -2);
    svint32_t vfb = svdup_n_s32_x(tp, -3);

    svint32_t v3= svdup_n_s32_x(tp, 3);
    svint32_t v5= svdup_n_s32_x(tp, 5);
    svint32_t v15= svdup_n_s32_x(tp, 15);

    svint32_t vr;
    svbool_t pg;
    vr = svdiv_s32_z(tp, va, v3);
    vr = svmul_s32_z(tp, vr, v3);
    pg = svcmpeq_s32(tp, va, vr);
    svst1_s32(pg, a.data()+s, vf);

    vr = svdiv_s32_z(tp, va, v5);
    vr = svmul_s32_z(tp, vr, v5);
    pg= svcmpeq_s32(tp, va, vr);
    svst1_s32(pg, a.data()+s, vb);

    vr = svdiv_s32_z(tp, va, v15);
    vr = svmul_s32_z(tp, vr, v15);
    pg= svcmpeq_s32(tp, va, vr);
    svst1_s32(pg, a.data()+s, vfb);
    s += w;
  }

  for(int i=0;i<n;i++){
    printf("%+03d ", a[i]);
  }
  printf("\n");
}
