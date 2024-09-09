#feature on safety
#include <std2.h>

int main() safe {
  std2::string s{"Hello safety"};

  // (B) - borrow occurs here.
  std2::string_view view = s;

  // (A) - invalidating action
  s = std2::string{"A different string"};

  // (U) - use that extends borrow
  std2::println(view);
}
