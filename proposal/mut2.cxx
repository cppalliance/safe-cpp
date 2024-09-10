#feature on safety
#include <std2.h>

int main() safe {
  std2::vector<size_t> A { }, B { };

  // Get a shared borrow to an element in B.
  // The borrow is live until it is loaded from below.
  const size_t^ b = B[0];

  // A is in the mutable context for its operator[] call.
  // B is not in the mutable conetxt for its operator[] call.
  // No borrow checker error.
  mut A[B[0]] += 1;

  // Keep b live.
  size_t x = *b;
}