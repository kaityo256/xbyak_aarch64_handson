#include <cstdio>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif

int main() {
  int n = 0;
#ifdef __ARM_FEATURE_SVE
  n = svcntb() * 8;
#endif
  if (n) {
    printf("SVE is available. The length is %d bits\n", n);
  } else {
    printf("SVE is unavailable.\n");
  }
}
