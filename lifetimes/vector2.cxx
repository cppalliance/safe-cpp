#feature on safety
#include <cstdint>

template<typename T>
class Vec {
public:
  void push_back(T value) % safe;

  const T% operator[](size_t idx) const % safe;
        T% operator[](size_t idx)       % safe;
};

void f2(Vec<float>% vec, float% x) safe {
  // Does push_back potentially invalidate x? 
  // No! Exclusivity prevents vec and x from aliasing.
  vec.push_back(7);

  // Okay to store to x, because it doesn't point into vec's data.
  *x = 7;
}

int main() safe {
  Vec<float> vec { };
  mut vec.push_back(1);

  // Ill-formed: shared borrow of vec between its mutable borrow and its use
  f2(mut vec, mut vec[0]);
}