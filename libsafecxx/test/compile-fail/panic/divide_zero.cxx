#feature on safety

int main() safe {
  // Panic to prevent undefined behavior with divide-by-zero.
  // This occurs regardless of safe context.
  int x = 20;
  int y = 0;
  int z = x / y;
}