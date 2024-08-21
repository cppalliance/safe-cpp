#feature on safety
#include <cstddef>

int main() safe {
  // Panic to prevent undefined behavior with int overflow.
  // This occurs regardless of safe context.
  int x = INT_MIN;
  int y = -1;
  int z = x / y;
}