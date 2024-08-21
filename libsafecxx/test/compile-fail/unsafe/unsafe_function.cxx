#feature on safety

int f();

int main() safe {
  // Ill-formed to call an unsafe function from a safe function.
  f();
}