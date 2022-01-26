#include <cstdio>
#include <vector>

int main() {
  // init
  const int n = 32;
  std::vector<int32_t> a(n);
  for (int i = 0; i < n; i++) {
    a[i] = i + 1;
  }
  // FizzBuzz
  for (int i = 0; i < n; i++) {
    uint32_t t = a[i];
    uint32_t r3 = (t / 3) * 3;
    if (r3 == t) {
      a[i] = -1;
    }
    uint32_t r5 = (t / 5) * 5;
    if (r5 == t) {
      a[i] = -2;
    }
    uint32_t r15 = (t / 15) * 15;
    if (r15 == t) {
      a[i] = -3;
    }
  }
  // Show Results
  for (int i = 0; i < n; i++) {
    if (a[i] == -1) {
      puts("Fizz");
    } else if (a[i] == -2) {
      puts("Buzz");
    } else if (a[i] == -3) {
      puts("FizzBuzz");
    } else {
      printf("%d\n", a[i]);
    }
  }
}
