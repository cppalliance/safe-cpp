#feature on safety
#include <std2.h>

int main() safe {
  std2::vector<size_t> A { }, B { };

  // Get a shared borrow to an element in B.
  // The borrow is live until it is loaded from below.
  const size_t^ b = B[0];

  // A[0] is in the mutable context, so A[0] is the mutable operator[].
  // B[0] is outside the mutable context, so B[0] is the const operator[].
  // No borrow checking error.
  mut A[0] += B[0];

  // Keep b live.
  size_t x = *b;
}