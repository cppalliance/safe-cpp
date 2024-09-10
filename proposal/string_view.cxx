#feature on safety
#include <std2.h>

using namespace std2;

int main() safe {
  // Populate the vector with views with a nice mixture of lifetimes.
  vector<string_view> views { };

  // string_view with /static lifetime.
  mut views.push_back("From a string literal");

  // string_view with outer scope lifetime.
  string s1("From string object 1");
  mut views.push_back(s1);

  {
    // string_view with inner scope lifetime.
    string s2("From string object 2");
    mut views.push_back(s2);

    // s2 goes out of scope. views now holds dangling pointers into
    // out-of-scope data.
  }

  // Print the strings. s2 already fell out of scope, so this should
  // be a borrowck violation. `views` now contains objects that hold
  // dangling pointers.
  println("Printing from the outer scope:");
  for(string_view sv : views)
    println(sv);
}
