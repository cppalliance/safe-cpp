#feature on safety

#include <cstdint>

int main() safe {
  int data1[2] { };
  int data2[2] { };

  int* p1 = data1;
  int* p2 = data2;

  // Undefined, because the pointers come from different allocations.
  ptrdiff_t diff = p2 - p1;
}