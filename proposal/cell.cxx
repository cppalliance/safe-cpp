#feature on safety
#include <std2.h>

using namespace std2;

int main() {
  // rc - Shared ownership within a thread.
  // ref_cell - Shared mutable access checked at runtime.
  rc<ref_cell<string>> s1(ref_cell<string>(string{"A string set from s1"}));

  // Copying the rc increments the reference counter on the control block.
  rc<ref_cell<string>> s2 = cpy s1;

  // Read the data out from s2.
  println(*s2->borrow());

  // The string data is now owned by three rcs.
  // cell's transactional access upholds the law of exclusivity.
  mut *s2->borrow_mut() =  string{"A string set from s2"};

  // Read out through s1.
  println(*s1->borrow());
}
