#include <vector>

void f1(std::vector<float>& vec, float& x) {
  // Do vec and x alias? If so, the push_back may invalidate x.
  vec.push_back(6);

  // Potential UB: x may have been invalidated by the push_back.
  x = 6;
}

int main() {
  std::vector<float> vec;
  vec.push_back(1):

  // Legacy references permit aliasing.
  f1(vec, vec[0]);
}