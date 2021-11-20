#include <arm_sve.h>
#include <cstdio>
#include <iostream>
#include <vector>

void svshow(svfloat64_t va) {
  int n = svcntd();
  std::vector<double> a(n);
  svbool_t tp = svptrue_b64();
  svst1_f64(tp, a.data(), va);
  for (int i = 0; i < n; i++) {
    printf("%+.7f ", a[n - i - 1]);
  }
  printf("\n");
}

void load_pat() {
  int n = svcntd();
  std::vector<double> a(n);
  for (int i = 0; i < n; i++) {
    a[i] = (i + 1);
  }
  svbool_t tp = svptrue_b64();
  svfloat64_t va = svld1_f64(tp, a.data());
  svbool_t tp2 = svptrue_pat_b64(SV_VL2);
  svfloat64_t vb = svld1_f64(tp2, a.data());
  std::cout << "va = " << std::endl;
  svshow(va);
  std::cout << "vb = " << std::endl;
  svshow(vb);
}

void add_all(svfloat64_t va, svfloat64_t vb) {
  svfloat64_t vc = svadd_f64_z(svptrue_b64(), va, vb);
  printf("va + vb (ALL) = ");
  svshow(vc);
}

int main() {
  double a[] = {0, 1, 2, 3, 4, 5, 6, 7};
  svfloat64_t va = svld1_f64(svptrue_b64(), a);
  printf("va = ");
  svshow(va);

  double b[] = {1, 1, 1, 1, 1, 1, 1, 1};
  svfloat64_t vb = svld1_f64(svptrue_b64(), b);
  printf("vb = ");
  svshow(vb);
  printf("\n");

  printf("Add all\n");
  printf("svadd_f64_z(svptrue_b64(), va, vb)\n");
  svfloat64_t vc1 = svadd_f64_z(svptrue_b64(), va, vb);
  printf("va + vb = ");
  svshow(vc1);
  printf("\n");

  printf("pattern VL2 with zero clear\n");
  printf("svadd_f64_z(svptrue_pat_b64(SV_VL2), va, vb)\n");
  svfloat64_t vc2 = svadd_f64_z(svptrue_pat_b64(SV_VL2), va, vb);
  printf("va + vb = ");
  svshow(vc2);
  printf("\n");

  printf("pattern VL2 with merging the first input\n");
  printf("svadd_f64_m(svptrue_pat_b64(SV_VL2), va, vb)\n");
  svfloat64_t vc3 = svadd_f64_m(svptrue_pat_b64(SV_VL2), va, vb);
  printf("va + vb = ");
  svshow(vc3);
  printf("\n");
}
