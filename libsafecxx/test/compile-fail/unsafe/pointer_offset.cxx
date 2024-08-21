#feature on safety

int main() safe {
  int data[10] { };
  int* p = data;

  // Illegal to offset a pointer.
  // This would be UB.
  int* p2 = p + 12;

  // Illegal to subscript a pointer, because of bounds safety.
  int x = p[30];
}