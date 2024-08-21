#feature on safety
#include <cstdint>

int main() safe {
  int array[4] { };
  
  // Stardard conversion from array borrow to slice borrow.
  [int; dyn]^ slice = ^array;

  // The slice borrow is the size of 2 pointers. It contains:
  // 1. A pointer to the data.
  // 2. A size_t of the elements after the pointer.
  static_assert(2 * sizeof(void*) == sizeof slice);

  // Panic on out-of-bounds slice subscript.
  size_t index = 10;
  int x = slice[index];
}