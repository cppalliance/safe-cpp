#feature on safety
#include <std2.h>

int main() safe {
  // p is uninitialized.
  std2::box<std2::string_view> p;

  // Error: p is uninitialized.
  println(*p);

  // p is definitely initialized.
  p = std2::box<std2::string_view>("Hello Safety");

  // Ok.
  println(*p);

  // p is moved into q. Now p is uninitialized again.
  auto q = rel p;

  // Error: p is uninitialized.
  println(*p);
}