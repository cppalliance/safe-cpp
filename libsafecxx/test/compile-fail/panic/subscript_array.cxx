#feature on safety
#include <cstdint>

int main() safe {
  int array[4] { };
  size_t index = 10;

  // Panic on out-of-bounds array subscript.
  int x = array[index];
}