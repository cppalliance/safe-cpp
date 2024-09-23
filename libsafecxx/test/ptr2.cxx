#feature on safety

template<typename T>
T call(T t) { return t; }

int^(*pf)(int^) = call<int^>;

int main() safe {}
