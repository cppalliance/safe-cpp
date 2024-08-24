# The `safe` context

Most builtin C++ operations are safe to use. They won't exhibit undefined behavior. Some operations may exhibit undefined behavior. If they can be checked by the compiler, either at compile time or during runtime using panic functions, that's great, they're still considered safe. But operations that may result in undefined that aren't automatically are _unsafe_.

Dereferencing a pointer or legacy reference (i.e., an lvalue or rvalue reference) is unsafe, because without costly dynamic instrumentation (the stuff of sanitizers), the program can't tell if the value referenced at that address is still initialized or not. 

Dereferencing a pointer is unsafe because it might hold a null pointer, and according to C++, it's undefined behavior to dereference a null pointer. But won't dereferencing a null pointer just segfault, rather than cause UB? Well, hopefully, but not necessarily. 

Code generators like LLVM work wit the concept of _poisoned values_.[^poison] A poison value represents a state that violates an assumption of the language. In C++, signed integer multiplication or addition that overflows or underflows is undefined behavior.[^ub] Compilers indicate a poison value for these results. In LLVM, the poison value for signed integer overflow is called `nsw` for "no signed wrap".[^nsw] Even though dereferencing a null pointer or adding integers that wraparound is defined by the hardware, programming languages intervene and make them undefined by indicating poison values. This lends backends the flexibility to reorder, reduce and refactor instructions and generate more optimized code.

Making C++ a memory safe language means reconsidering which operations are poisoned. If an operation remains poisoned, it should either be _unsafe_ or it should signal the panic function if detected at runtime. The choice can be a bit arbitrary. I expect the compiler to support different behaviors depending on user's needs.

For Circle, these are the defaults I've chosen for safe operations that may panic:

* Integer divide or modulus by 0 is tested at runtime and panics.
* Signed integer divide or modulus by -1 when the numerator is MIN and the denominator is -1. This is tested at runtime and panics.
* Out-of-bounds subscripts into arrays and slices is tested at runtime and panics.

These operations are prohibited in the safe context:

* Dereferencing pointers and legacy references.
* Subscripts into pointers.
* Pointer offset and pointer difference are unsafe due to poison when indexing outside the allocation.
* `reinterpret_cast` and pointer/reference downcast operations.
* `dynamic_cast`. A safe equivalent is planned.
* Accessing members of `union` types is unsafe. Some safe usage is planned, for when all alternatives have the same type, which is common when implementing mathematical vectors.
* Naming global non-const objects in an evaluated context. This is to prevent data races. However, preventing the static initialization order fiasco[^siof] will entail being more restrictive even with const global objects.
* Calling unsafe functions is unsafe, because you have to assume that the unsafe functions are doing unsafe things.

Undefined in C++, but defined and safe in Circle:

* Signed integer overflow is defined according to two's complement. That is, the `nsw` and `nuw` poison values are never asserted. Some optimization opportunities are lost, but you save paying a runtime check.

This lists largely corresponds with Rust's unsafe restictions.[^rust-unsafe] I encourage you to read the Rustnomicon[^rustnomicon] for a thoughtful explanation of the mental model for safe and unsafe contexts.

Understand that the `safe` context is not actually what makes your code safe. The goal is to reduce your exposure to unsafe APIs, which may unwittingly lead to undefined behavior. The only thing the `safe` context does is error when you call unsafe APIs or use unsafe operations. If you have code that builds, you can remove `safe` everywhere and it will continue to be just as sound.

## _safe-specifier_

Similar to the _noexcept-specifier_,[^noexcept-spec] function types and declarations may be marked with a _safe-specifier_. Place this after the _noexcept-specifier_. 

```cpp
// `safe` is part of the function type.
using F1 = void(int);
using F2 = void(int) safe;
using F3 = void(int) noexcept;
using F4 = void(int) noexcept safe;
```

`safe` is part of the function's type, so function's with different _safe-specifiers_ always compare differently.


```cpp
// `safe` is part of the function type.
static_assert(F1 != F2);
static_assert(F3 != F4);
```

As with _noexcept-specifier_, the safeness of a function's type can be stripped when converting function pointers. It's unsound to add a _safe-specifier_ during conversion, so that's prohibit. But it's okay to strip `safe`, just as it's permitted to strip `noexcept`.

```cpp
// You can strip off `safe` in function pointers.
static_assert(std::is_convertible_v<F2*, F1*>);
static_assert(std::is_convertible_v<F4*, F3*>);

// You can strip off both `noexcept` and `safe`.
static_assert(std::is_convertible_v<F4*, F1*>);

// You can't add safe. That's unsafe.
static_assert(!std::is_convertible_v<F1*, F2*>);
static_assert(!std::is_convertible_v<F3*, F4*>);
```

Declaring functions with value-dependent _safe-specifiers_ is supported, although I haven't found a strong motivation for doing this. Right now, it's there for completeness.

```cpp
template<bool IsSafe>
struct foo_t {
  // Value-dependent safe-specifier
  void func() safe(IsSafe);
};
```

## _safe-expression_

You can query the safeness of an expression in an unevaluated context with the _safe-expression_. It's analagous to the existing _noexcept-expression_.[^noexcept-expr] This is very useful when paired with _requires-clause_,[^requires-clause] as it lets you constrain inputs based on the safeness of a callable.

```cpp
#feature on safety

template<typename F, typename... Args>
void spawn(F f, Args... args) safe requires(safe(f(args...)));

struct Foo {
  // The int overload is safe.
  void operator()(const self^, int) safe;

  // The double overload is unsafe.
  void operator()(const self^, double);
};

int main() safe {
  Foo obj { };
  spawn(obj, 1);   // OK
  spawn(obj, 1.1); // Ill-formed. Fails requires-clause.
}
```

Consider a `spawn` function that takes a callable `f` and a set of arguments `args`. The function is marked `safe`. Transitively, the callable, when invoked with the provided arguments, must also be a safe operation. But we can't stipulate `safe` on the type `F`, because it may not be a function type. In this example it's a class with two overloaded call operators.

When the user provides an integer argument, the _requires-clause_ substitutes to `safe(f(1))`, which is true, because the best viable candidate for the function call is `void operator()(const self^, int) safe;`. That's a safe function.

When the user provides a floating-point argument, the _requires-clause_ substitutes to `safe(f(1.1))`, which is false, because the best viable candidate is `void operator()(const self^, double);`. That's not a safe function.

These kind of constraints are idiomatic in C++ but not found, or even supported, in Rust, which uses early-checked traits to provide generics.

## Panic functions

### Integer division

To stop undefined behavior, the compiler checks integer arithmetic for division/modulus by 0 and panics. For signed integers, division/modulus of the min value by -1 would cause an unrecoverable overflow. That's also checked and panics.

```cpp
#feature on safety
#include <cstdio>
#include <cstdlib>
#include <climits>

int make_div(int num, int den) safe {
  // Check for UB and panic. Doesn't matter if function is `safe` or not.
  return num / den;
}

int main(int argc, char** argv) {
  int num = atoi(argv[1]);
  int den = atoi(argv[2]);
  int x = make_div(num, den);
  printf("%d / %d = %d\n", num, den, x);
}
```
```txt
$ ./div 50 5
50 / 5 = 10

$ ./div 10000 0
div.cxx:8:14
int make_div(int, int) safe
cannot divide integer type int by 0
Aborted (core dumped)

$ ./div -2147483648 -1
div.cxx:8:14
int make_div(int, int) safe
cannot divide type int -2147483648 by -1
Aborted (core dumped)
```

Voilà. The program panics, with an informative message, rather than failing with a hardware exception.

### Bounds checking










TODO

## unchecked specifier

TODO

[^poison]: [Poison values](https://llvm.org/docs/LangRef.html#poisonvalues)

[^ub]: [Undefined behavior](https://en.cppreference.com/w/cpp/language/ub)

[^nsw]: [No signed wrap](https://llvm.org/docs/LangRef.html#id91)

[^siof]: [Static initialization order fiasco](https://en.cppreference.com/w/cpp/language/siof)

[^rust-unsafe]: [What Unsafe Rust Can Do](https://doc.rust-lang.org/nomicon/what-unsafe-does.html)

[^rustnomicon]: [Rustnomicon](https://doc.rust-lang.org/nomicon/intro.html)

[^noexcept-spec]: [Noexcept specifier](https://en.cppreference.com/w/cpp/language/noexcept_spec)

[^noexcept-expr]: [Noexcept operator](https://en.cppreference.com/w/cpp/language/noexcept)

[^requires-clause]: [Requires clauses](https://en.cppreference.com/w/cpp/language/constraints#Requires_clauses)