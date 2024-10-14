#feature on safety

class string_view/(a) {
  // Keep a borrow to a slice over the string data.
  const [char; dyn]^/a p_;
public:
};

class info/(a) {
  // The handle has lifetime /a.
  string_view/a sv;

public:
  void swap(self^, info^ rhs) safe {
    string_view temp = self->sv;
    self->sv = rhs->sv;
    rhs->sv = temp;
  }
};

void func/(a)(info/a^ lhs, info/a^ rhs) safe {
  lhs.swap(rhs);
}

void func2(info^ lhs, info^ rhs) safe {
  lhs.swap(rhs);
}