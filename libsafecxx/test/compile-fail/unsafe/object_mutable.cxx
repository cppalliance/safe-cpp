#feature on safety

struct Object {
  mutable int member;

  void mutate() const safe {
    // Unsafe to name a mutable data member from a const expression.
    // This would break the law of exclusivity.
    ++member;
  }
};

int main() safe {
  const Object obj { 1 };
  obj.mutate();
  
  obj.member++;
}

