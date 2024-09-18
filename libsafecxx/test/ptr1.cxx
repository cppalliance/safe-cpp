#feature on safety

template<typename T>
void call/(where T:static)(T) { }

void func1(int^/static ref) {
  // Should work for calls with a lifetime parameter.
  call(ref);
}

void func2(int x) {
  // Should work for calls without a lifetime parameter.
  // In this case, call's instantiation does not have SCCs.
  call(x);
}

int main() {
  // This had been breaking before build_sep_18_2024-1.
  void(*pf)(int) = addr call<int>;
}