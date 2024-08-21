#feature on safety

int main() safe {
  int* p = nullptr;

  // Unsafe to dereference a legacy pointer.
  int x = *p;
}