#feature on safety
#include <std2.h>

int main() safe {
  std2::vector<const int^> vec { };
  {
    int x = 101;
    mut vec.push_back(x);
  }

  // Okay. vec has dangling borrows, but passes the drop check.
  drp vec;
}