#feature on safety

int global = 1;

int main() safe {
  // Unsafe. Potential data race if one thread is writing to the global
  // and another is reading it.
  ++global;
}