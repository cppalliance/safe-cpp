#feature on safety
#include <std2.h>

using namespace std2;

int main() safe {
  initializer_list<string_view> initlist;
  
  string s = "Hello";
  string t = "World";

  // initializer lists holds dangling pointers into backing array.
  initlist = { s, t, s, t };
  
  // Borrow checker error. `use of initlist depends on expired loan`
  vector<string_view> vec(rel initlist);
  for(string_view sv : vec)
    println(sv);
}