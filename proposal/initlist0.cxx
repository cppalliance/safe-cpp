#include <vector>
#include <string_view>
#include <iostream>

using namespace std;

int main() {
  initializer_list<string_view> initlist1;
  initializer_list<string_view> initlist2;
  
  string s = "Hello";
  string t = "World";

  // initializer lists holds dangling pointers into backing array.
  initlist1 = { s, s, s, s };
  initlist2 = { t, t, t, t };
  
  // Prints like normal.
  vector<string_view> vec(initlist1);
  for(string_view sv : vec)
    cout<< sv<< "\n";

  // Catastrophe.
  vec = initlist2;
  for(string_view sv : vec)
    cout<< sv<< "\n";
}