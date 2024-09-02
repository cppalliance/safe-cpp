#feature on safety

template<typename T+>
struct Vec {
  Vec() safe;
  void push_back(self^, T obj) safe;

  template<typename... Ts>
  void emplace_back(self^, Ts... obj) safe {
    // Direct initialization with the String(const char*) ctor.
    // This compiles, because T is unsafe-qualified.
    self.push_back(T(rel obj...));
  }
};

struct String {
  String(const char*); // Unsafe ctor.
};

int main() safe {
  // Vec has a safe constructor.
  Vec<unsafe String> vec { };

  // void Vec<unsafe String>::emplace_back(self^, const char*) safe;
  mut vec.push_back("A string");
}