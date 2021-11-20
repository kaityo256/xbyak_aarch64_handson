#include <arm_sve.h>
#include <iostream>
#include <vector>

void show_ppr(svbool_t tp) {
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

void ptrue() {
  std::cout << "# pture samples for various types" << std::endl;
  std::cout << "svptrue_b8" << std::endl;
  show_ppr(svptrue_b8());
  std::cout << "svptrue_b16" << std::endl;
  show_ppr(svptrue_b16());
  std::cout << "svptrue_b32" << std::endl;
  show_ppr(svptrue_b32());
  std::cout << "svptrue_b64" << std::endl;
  show_ppr(svptrue_b64());
}

void ptrue_pat() {
  std::cout << "# pture_pat samples for vrious patterns" << std::endl;
  std::cout << "svptrue_pat_b8(SV_ALL)" << std::endl;
  show_ppr(svptrue_pat_b8(SV_ALL));
  std::cout << "svptrue_pat_b8(SV_VL1)" << std::endl;
  show_ppr(svptrue_pat_b8(SV_VL1));
  std::cout << "svptrue_pat_b8(SV_VL2)" << std::endl;
  show_ppr(svptrue_pat_b8(SV_VL2));
  std::cout << "svptrue_pat_b8(SV_VL3)" << std::endl;
  show_ppr(svptrue_pat_b8(SV_VL3));
  std::cout << "svptrue_pat_b8(SV_VL4)" << std::endl;
  show_ppr(svptrue_pat_b8(SV_VL4));
}

void ptrue_pat_types() {
  std::cout << "# pture_pat samples for various types" << std::endl;
  std::cout << "svptrue_pat_b8(SV_VL2)" << std::endl;
  show_ppr(svptrue_pat_b8(SV_VL2));
  std::cout << "svptrue_pat_b16(SV_VL2)" << std::endl;
  show_ppr(svptrue_pat_b16(SV_VL2));
  std::cout << "svptrue_pat_b32(SV_VL2)" << std::endl;
  show_ppr(svptrue_pat_b32(SV_VL2));
  std::cout << "svptrue_pat_b64(SV_VL2)" << std::endl;
  show_ppr(svptrue_pat_b64(SV_VL2));
}

int main() {
  ptrue();
  std::cout << "----" << std::endl;
  ptrue_pat();
  std::cout << "----" << std::endl;
  ptrue_pat_types();
}
