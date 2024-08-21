#feature on safety

#include <std2/vector.h>

int main() {
  std2::slice_iterator<int> it;
  {
    std2::vector<int> vec = { 1, 2, 3, 4, 5 };
    it = vec^.iter();
  }
  it^.next();
}
