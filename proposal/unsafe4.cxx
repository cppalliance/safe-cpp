#feature on safety

template<typename T+>
struct Vec {
  Vec() safe;
  void push_back(self^, T obj) safe;
};

struct String {
  String(const char*); // Unsafe ctor.
};

int main() safe {
  // Vec has a safe constructor.
  Vec<unsafe String> vec { };

  // void Vec<unsafe String>::push_back(self^, unsafe String) safe;
  // This is ill-formed. We can't invoke the unsafe String constructor
  // to initialize an unsafe type.
  mut vec.push_back(String("A string"));
}