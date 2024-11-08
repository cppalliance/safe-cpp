#feature on safety

template<typename Key, typename Value>
class Map {
public:
  // Non-static member functions do not constrain the result object to
  // the function parameters.
  auto get1(const Key% key) % safe -> Value%;

  // Free function do constrain the result object to the f unction praameters.
  auto get2(self%, const Key% key) safe -> Value%;
};

int main() safe {
  Map<float, long> map { };

  // Bind the key reference to a materialized temporary.
  // The temporary expires at the end of this statement.
  long% value1 = mut map.get1(3.14f);

  // We can still access value, because it's not constrained on the 
  // key argument.
  *value1 = 1001;

  // The call to get2 constrains the returned reference to the lifetime
  // of the key temporary.
  long% value2 = mut map.get2(1.6186f);

  // This is ill-formed, because get2's key argument is out of scope.
  *value2 = 1002;
}