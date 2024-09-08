#feature on safety
#include <std2.h>          // Pull in the definition of std2::tuple.

using T0 = ();             // Zero-length tuple type.
using T1 = (int, );        // One-length tuple type.
using T2 = (int, double);  // Longer tuples type.

// Nest at your leisure.
using T3 = (((int, double), float), char);

int main() {
   // Zero-length tuple expression.
  auto t0 = (,);
  static_assert(T0 == decltype(t0));

  // One-length tuple expression.
  auto t1 = (4, );
  static_assert(T1 == decltype(t1));
     
  // Longer tuple expression.
  auto t2 = (5, 3.14);
  static_assert(T2 == decltype(t2));

  // Nest tuples.
  auto t3 = (((1, 1.618), 3.3f), 'T');
  static_assert(T3 == decltype(t3));

  // Access the 1.618 double field:
  auto x = t3.0.0.1;
  static_assert(double == decltype(x));
}