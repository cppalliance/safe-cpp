#feature on safety
#include <std2.h>
#include <string>

int main() safe {
  // Requires unsafe type specifier because std::string's dtor is unsafe.
  std2::vector<unsafe std::string> vec { };

  // Construct an std::string from a const char* (unsafe)
  // Pass by relocation (unsafe)
  mut vec.push_back("Hello unsafe type qualifier!");

  // Append Bar to the end of Foo (unsafe)
  mut vec[0] += "Another unsafe string";

  std2::println(vec[0]);
}