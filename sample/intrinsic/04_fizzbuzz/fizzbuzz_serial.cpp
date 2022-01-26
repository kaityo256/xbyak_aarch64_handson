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
    if (a[i] % 15 == 0) {
      a[i] = -3;
    } else if (a[i] % 3 == 0) {
      a[i] = -1;
    } else if (a[i] % 5 == 0) {
      a[i] = -2;
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
