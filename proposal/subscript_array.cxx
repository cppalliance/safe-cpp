#feature on safety
#include <cstdint>

int main() safe {
  int array[4] { 1, 2, 3, 4 };
  size_t index = 10;

  // Panic on out-of-bounds array subscript.
  return array[index];
}