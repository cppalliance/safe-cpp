#feature on safety

void f(int% x, int% y) safe;

void g(int& x, int& y) safe {
  unsafe {
    // Enter an unsafe block to dereference legacy references.
    // The precondition to the unsafe-block is that the legacy
    // references *do not alias* and *do not dangle*.
    f(%*x, %*y);
  }
}

void f(int% x, int% y) safe {
  // We can demote safe references to legacy references without 
  // an unsafe block. The are no preconditions to enforce.
  g(&*x, &*y);
}