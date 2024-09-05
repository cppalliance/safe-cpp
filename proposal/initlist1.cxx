#feature on safety
#include <std2.h>

using namespace std2;

int main() safe {
  initializer_list<string_view> initlist1;
  initializer_list<string_view> initlist2;
  
  string s = "Hello";
  string t = "World";

  // initializer lists holds dangling pointers into backing array.
  initlist1 = { s, s, s, s };
  initlist2 = { t, t, t, t };
  
  // Borrow checker error. `use of initlist1 depends on expired loan`
  vector<string_view> vec(rel initlist1);
  for(string_view sv : vec)
    println(sv);

  // Borrow checker error. `use of initlist2 depends on expired loan`
  vec = rel initlist2;
  for(string_view sv : vec)
    println(sv);
}