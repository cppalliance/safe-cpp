#feature on safety
#include <std2.h>
#include <string>

int main() safe {
  // Requires unsafe type specifier because std::string's dtor is unsafe.
  std2::vector<unsafe std::string> vec { };

  // Construct an std::string from a const char* (unsafe)
  // Pass by relocation (unsafe)
  mut vec.push_back("Hello unsafe type qualifier!");

  // Pass const char*
  // Construct inside emplace_back (unsafe)
  mut vec.push_back("I integrate with legacy types");

  // Append Bar to the end of Foo (unsafe)
  mut vec[0] += vec[1];

  std2::println(vec[0]);
}