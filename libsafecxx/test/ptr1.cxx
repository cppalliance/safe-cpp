#feature on safety

template<typename T>
void call/(where T:static)(T) { }

template<class T>
void call2/(a, where T: a)(int^/a, T) {}

void func1(int^/static ref) {
  // Should work for calls with a lifetime parameter.
  call(ref);

  int x = 1234;
  call2(^x, ref);
}

void func2(int x) {
  // Should work for calls without a lifetime parameter.
  // In this case, call's instantiation does not have SCCs.
  call(x);
}

int main() {
  // This had been breaking before build_sep_18_2024-1.
  void(*pf)(int) = addr call<int>;
  // void(*pf2)(int^/static) = addr call<int^/static>;
}
