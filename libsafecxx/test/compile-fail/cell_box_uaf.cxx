// A challenging type relation test.
// This uses the new region typing implementation.
// Requires build `sep_12_2024-1-variance.tgz` or later.

#feature on safety
#include <std2.h>

using namespace std2;

struct S/(a)
{
  cell<optional<S/a const^/a>> x_;
  box<int> p_;

  S(box<int> p) safe
    : x_(optional<S/a const^/a>(.none))
    , p_(rel p)
  {
  }

  ~S() safe {
  }
};

int main() safe
{
  // shouldn't compile
  {
    S s1(box<int>(1234));
    S s2(box<int>(4321));
    s1.x_.set(.some(^const s2));
    s2.x_.set(.some(^const s1));
  }
}
