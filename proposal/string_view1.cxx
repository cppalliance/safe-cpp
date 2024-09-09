#feature on safety
#include <std2.h>

int main() safe {
  std2::string s("Hellooooooooooooooo ");
  std2::string_view sv = s + "World\n";
  println(sv);
}