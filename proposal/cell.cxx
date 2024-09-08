#feature on safety
#include <std2.h>

using namespace std2;

int main() {
  // rc - Shared ownership within a thread.
  // cell - Transactional access.
  rc<cell<string>> s1(string("My shared string"));

  // Copying the rc increments the reference counter on the control block.
  rc<cell<string>> s2 = cpy s1;
  rc<cell<string>> s3 = cpy s1;

  // Read out from s2.
  println(s2->get());

  // The string data is now owned by three rcs. 
  // cell's transactional access upholds the law of exclusivity.
  s2->set("A string set through s2");

  // Read out through s3.
  println(s3->get());

  // Load the value from s3...
  string s = s3->get();

  // Append.
  mut s.append(" -- Appending more text!");

  // Store into s2.
  s2->set(rel s);

  // Read out through s1.
  println(s1->get());
}