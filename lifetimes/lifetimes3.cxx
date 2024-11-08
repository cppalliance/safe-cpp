#feature on safety

const int% f1(const int% x, const int% y, bool pred) safe {
  // The return reference is constrained by all reference parameters: x and y.
  return pred ? x : y;
}

struct Obj {
  const int% f2(const int% arg) const % safe {
    // Non-static member functions are constrained by the implicit 
    // object lifetime.
    // It's OK to return `x`, because self outlives the return.
    return %x;
  }

  const int% f3(const int% arg) const % safe {
    // Error: arg does not outlive the return reference.
    return arg;
  }

  const int% f4(const self%, const int% arg) safe {
    // OK - f4 is a free function with an explicit self parameter.
    return arg;
  }

  int x;
};

int main() {
  int x = 1, y = 2;
  f1(x, y, true); // OK

  Obj obj { };
  obj.f2(x);  // OK
  obj.f3(x);  // Error
  obj.f4(x);  // OK.
}