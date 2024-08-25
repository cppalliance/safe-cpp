#feature on safety

void f1() safe {
  int x = 0;
  int* p = &x;
  unsafe {
    // These unsafe operations are permitted in a safe context.
    int y = *p;
    ++p;
    p++;
    p = p + 1;
    bool pred = p < p;
  }
}

void f2() safe {
  int x = 0;
  int* unsafe p = &x;

  // These unsafe operations are permitted on safe pointer types. 
  int y = *p;
  ++p;
  p++;
  p = p + 1;
  bool pred = p < p;
}

void f3() safe {
  int x = 0;
  int* p = &x;

  // These unsafe operations are make the program ill-formed.
  int y = *p;
  ++p;
  p++;
  p = p + 1;
  bool pred = p < p;
}