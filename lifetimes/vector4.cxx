#feature on safety

template<typename Key, typename Value>
class Map {
public:
  // Lifetime elision rules constrain the return by self.
  auto get1(self^, const Key^ key) safe -> Value^;

  // Use explicit parameterizations for alternate constraints.
  auto get2/(a)(self^/a, const Key^/a key) safe -> Value^/a;
};

int main() safe {
  Map<float, long> map { };

  // Bind the key reference to a materialized temporary.
  // The temporary expires at the end of this statement.
  long^ value1 = mut map.get1(3.14f);

  // We can still access value, because it's not constrained on the 
  // key argument.
  *value1 = 1001;

  // The call to get2 constrains the returned reference to the lifetime
  // of the key temporary.
  long^ value2 = mut map.get2(1.6186f);
  
  // This is ill-formed, because get2's key argument is out of scope.
  *value2 = 1002;
}