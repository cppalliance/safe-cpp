#feature on safety

template<typename T+, bool DropOnly>
struct Vec {
  Vec() safe { }

  [[unsafe::drop_only(T)]] ~Vec() safe requires(DropOnly);
                           ~Vec() safe requires(!DropOnly);

  void push(self^, T rhs) safe;

  // T is a phantom data member of Vec. The non-trivial dtor will
  // use the lifetimes of T, raising a borrow checker error if
  // T is not drop_only.
  T __phantom_data;
};

template<typename T, bool DropOnly>
void test() safe {
  Vec<const T^, DropOnly> vec { };
  {
    T x = 101;
    mut vec.push(^x);
  }

  // Use of vec is ill-formed due to drop of x above.
  // int y = 102;
  // mut vec.push(^y);

  // Should be ill-formed due to drop check.
  drp vec;
}

int main() safe {
  test<int, true>();
  test<int, false>();
}