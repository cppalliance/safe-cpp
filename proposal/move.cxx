#feature on safety
#include <std2.h>

int main() safe {
  // Objects start off uninitialized. A use of an uninitialized
  // object is ill-formed.
  std2::string s;

  // Require explicit initialization.
  s = std2::string("Hello ");

  // Require explicit mutation.
  mut s.append("World");

  // Require explicit relocation.
  std2::string s2 = rel s;  // Now s is uninitialized.

  // Require explicit non-trivial copy.
  std2::string s3 = cpy s2;

  // `Hello World`.
  println(s3);
}