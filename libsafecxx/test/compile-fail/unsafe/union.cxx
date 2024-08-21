#feature on safety

union Variant {
  float f;
  double d;
  int i;
  long l;
};  

int main() safe {
  Variant v;
  v.d = 3.14;

  // TODO: Bug here. initialization analysis complains of 
  // uninitialized object v.f.
  // It's unsafe to access a variant member. This is likely to
  // violate type safety.
  float f = v.f;
}