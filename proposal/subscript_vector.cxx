#feature on safety
#include <std2.h>

int main() safe {
  std2::vector<int> vec { 1, 2, 3, 4 };
  size_t index = 10;

  // Panic on out-of-bounds vector subscript.
  int x = vec[index];
}