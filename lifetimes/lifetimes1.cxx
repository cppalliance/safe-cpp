#feature on safety

// Function parameters have different lifetime parameters. 
// Return type is constrained by x's lifetime.
auto f1/(a, b)(int^/a x, int^/b y, bool pred) safe -> int^/a {
  // Error:
  // function auto f1/(a, b)(int^/a, int^/b) -> int^/a returns
  // object with lifetime b, but b doesn't outlive a
  // return y;
  return pred ? x : y;
}

// Function parameters have a common lifetime parameter.
auto f2/(a)(int^/a x, int^/a y, bool pred) safe -> int^/a {
  // Ok
  return pred ? x : y;
}

// Error:
// cannot use lifetime elision for return type int^ 
auto f3(int^ x, int^ y) safe -> int^;
