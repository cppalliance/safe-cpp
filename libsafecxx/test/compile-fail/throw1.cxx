#feature on safety
#include <std2.h>

int main() {
  using namespace std2;

  try {
    string s("A stack string");
    string_view sv = s;

    // Error: s constrained to live as long as static, but s does not live
    // that long.
    throw sv;

  } catch(string_view sv) {
    println(sv);
  }
}