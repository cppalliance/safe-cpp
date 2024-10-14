#feature on safety

// New elision rules:
// All parameters are constrained by a common lifetime.
// The common lifetime constrains the return type.
int% f4(int% x, int% y, bool pred) safe {
  // Can return either x or y, because they outlive the common lifetime
  // and the common lifetime outlives the result object.
  return pred ? x : y;
}
