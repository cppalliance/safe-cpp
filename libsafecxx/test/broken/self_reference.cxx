#feature on safety

template<class T+>
choice optional {
  some(T),
  default none;
};

struct X/(a) {
  mutable int x;
  optional<X const^/a> b;
};

int main()
{
  X x{1234, .none};
  x.b = .some(^x);
}
