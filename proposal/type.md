# Type safety

## Relocation object model

A core enabling feature of Safe C++ is the new object model. It supports relocation/destructive move of local objects, which is necessary for satisfying type safety. Additionally, _all mutations are explicit_. This is nice in its own right, but it's really important for distinguishing between mutable and shared borrows.

```cpp
#feature on safety
#include "std2.h"

using namespace std2;

int main() safe {
  // No default construct. p is uninitialized.
  unique_ptr<string> p;

  // Ill-formed: p is uninitialized.
  println(*p);
}
```
```txt
$ circle unique1.cxx 
safety: unique1.cxx:11:12
  println(*p); 
           ^
cannot use uninitialized object p
```

The `std2::unique_ptr` has no default state. It's safe against [null pointer type safety bugs](intro.md#2-type-safety-null-variety). A `unique_ptr` that's declared without a constructor is not default initialized. It's uninitialized. It's illegal to use until it's been assigned to.

```cpp
#feature on safety
#include "std2.h"

using namespace std2;

void f(unique_ptr<string> p) safe { }

int main() safe {
  // No default construct. p is uninitialized.
  unique_ptr<string> p;
  p = unique_ptr<string>::make("Hello");
  println(*p);

  // Relocate to another function.
  f(rel p);

  // Ill-formed: p is uninitialized.
  println(*p);
}
```
```txt
$ circle unique2.cxx 
safety: unique2.cxx:18:12
  println(*p);  // Ill-formed. 
           ^
cannot use uninitialized object p
```

Once we assign to `p`, we can use it. But we can't `std::move` into another function, because move semantics put the operand into its default state, and we're stipulating that the unique_ptr has no default state. Fortunately, the new object model provides for _relocation_, which moves the contents of the object into a value, and sets the source to uninitialized. The destructor on the old object never gets run, because that old declaration no longer owns the object. Ownership has changed with the relocation.

It's a feature, not a defect, that the compiler errors when you use an uninitialized or potentially uninitialized object. The alternative is a type safety error, such as a null pointer dereference undefined behavior.

In Rust, objects are _relocated by default_, unless they implement the `Copy` trait,[^copy-trait] in which case they're copied. If you want to copy a non-Copy object, implement the `Clone` trait[^clone-trait] and call the `clone` member function.

I think implicit relocation is too surprising for C++ users. We're more likely to have raw pointers and legacy references tracking objects, and you don't want to pull the storage out from under them, at least not without some clear token in the source code. That's why Circle includes _rel-expression_ and _cpy-expression_.

* `rel x` - relocate `x` into a new value. `x` is set as uninitialized.
* `cpy x` - copy construct `x` into a new value. `x` remains initialized.

If an object is _trivially constructible_, then you don't need either of these tokens. The compiler will copy your value. Both expressions produce prvalues of the lvalue operand type. 

Why do I make both copy and relocation explicit? I want to make it easy for users to choose the more efficient option. If a type is not trivially copyable, you can opt into an expensive copy with _cpy-expression_. This avoids performance bugs, where an object undergoes an expensive copy just because the user didn't know it was there. Or, if you don't want to copy, use _rel-expression_, which is efficient but destroys the old object, without destructing it.

* `drp x` - call the destructor on an object and set as uninitialized.

Local objects start off uninitialized. They're initialized when first assigned to. Then they're uninitialized again when relocated from. If you want to _destruct_ an object prior to it going out of scope, use _drp-expression_. Unlike Rust's `drop` API,[^drop] this works even on objects that are only potentially initialized (was uninitialized on some control flow paths) or partially initialized (has some uninitialized subobjects).

Consider a function like `std::unique_ptr::reset`.[^unique_ptr-reset] It destructs the existing object, if one is engaged, and sets the unique_ptr to its null state. But in our safe version, unique_ptr doesn't have a default state. It doesn't supply the `reset` member function. Instead, users just drop it, running its destructor and leaving it uninitialized.

You've noticed the nonsense spellings for these keywords. Why not call them `move`, `copy` and `drop`? I wanted to avoid shadowing those common identifiers and improve results when searching code or the web.

### Tuples

Using an uninitialized or potentially uninitialized object raises a compile time error. The compiler can only account for the state of local variables, and then only when they're indicated by name, and not by reference. All local objects and subobjects, which are declarations _owned by the function_, are assigned _place names_. The compiler provisions flags that hold the state for each place name. You can relocate and drop owned local objects by place name, because the compiler can locate their initialization flags. It's not possible for the compiler to chase through references and function calls to find initialization data for objects potentially owned by other functions.

```cpp
#feature on safety
#include <tuple>

struct Pair {
  int x, y;
};

Pair g { 10, 20 };

int main() {
  // Relocate from an std::tuple element.
  auto tup = std::make_tuple(5, 1.619);
  int x = rel *get<0>(&tup);

  // Relocate from runtime subscript.
  int data[5] { };
  int index = 1;
  int y = rel data[index];

  // Relocate from subobject of a global.
  int gy = rel g.y;
}
```
```txt
$ circle rel1.cxx 
safety: rel1.cxx:13:15
  int x = rel *get<0>(&tup); 
              ^
rel operand does not refer to an owned place
an owned place is a local variable or subobject of a local variable
the place involves a dereference of int&

safety: rel1.cxx:18:19
  int y = rel data[index]; 
                  ^
rel operand does not refer to an owned place
an owned place is a local variable or subobject of a local variable
the place involves a subscript of int[5]

safety: rel1.cxx:21:17
  int gy = rel g.y; 
                ^
rel operand does not refer to an owned place
an owned place is a local variable or subobject of a local variable
g is a non-local variable declared at rel1.cxx:8:6
Pair g { 10, 20 }; 
     ^
```

This example demonstrates that you can't relocate through the reference returned from `std::get`, you can't relocate through a dynamic subscript of an array and you can't relocate the subobject of a global variable. You can only relocate or drop _owned places_.

If we can't relocate through a reference, how do we relocate through elements of `std::tuple`, `std::array` or `std::variant`? Unless special compiler support for these containers is implemented, you can't. Those standard containers only provide access to their elements through accessor functions. But initialization analysis that enables relocation only considers the definition of the current function; it doesn't leap into other functions, because that's the slippery slope to whole-program analysis.

Circle addresses the defects in C++'s algebraic types (which I find objectionable many other reasons too) by including new first-class tuple, array and [choice](#choice-types) types. Safe C++ is still fully compatible with legacy types, but because of their non-local element access, relocation from their subobjects is not feasible. Relocation is important to type safety, because many types prohibit default states, making C++-style move semantics impossible. Either relocate your object, or put it in an `optional` from which it can be unwrapped.

```cpp
#feature on safety
#include "std2.h"

using T0 = ();             // Zero-length tuple type.
using T1 = (int, );        // One-length tuple type.
using T2 = (int, double);  // Longer tuples type.

// Nest at your leisure.
using T3 = (((int, double), float), char);

int main() {
   // Zero-length tuple expression.
  auto t0 = (,);
  static_assert(T0 == decltype(t0));

  // One-length tuple expression.
  auto t1 = (4, );
  static_assert(T1 == decltype(t1));
     
  // Longer tuple expression.
  auto t2 = (5, 3.14);
  static_assert(T2 == decltype(t2));

  // Nest tuples.
  auto t3 = (((1, 1.618), 3.3f), 'T');
  static_assert(T3 == decltype(t3));

  // Access the 1.618 double field:
  auto x = t3.0.0.1;
  static_assert(double == decltype(x));
}
```

Circle implements special tuple syntax, _like every other modern language_. Types are noted with comma-separated lists of types inside parentheses. Expressions are noted with comma-separated lists of expressions inside parentheses. You can nest them. You can access elements of tuple expressions by chaining indices together with dots.

Use `circle -print-mir` to dump the MIR of this program.

```txt
   15  Assign _4.1 = 84
   16  Commit _4 (((int, double), float), char)
   17  InstLive _5 double
   18  Assign _5 = use _4.0.0.1
   19  InstDead _5
   20  InstDead _4
```

The assignment `t3.0.0.1` lowers to `_4.0.0.1`. This is a place name of a local variable. It's an _owned place_. The compiler would be able to relocate out of this place, because it doesn't involve dereferences, chasing function calls or accessing global state.

### Arrays and slices

### operator `rel`

TODO

## Explicit mutation

Reference binding convention is important in the context of borrow checking. Const and non-const borrows differ by more than just constness. Users are allowed multiple live shared borrows, but only one live mutable borrow. C++'s convention of always preferring non-const references would tie the borrow checker into knots, as mutable borrows don't permit aliasing. We want to bind shared borrows by default, and require a token to opt into binding mutable borrows.

To achieve this binding precision, the relocation object model takes a new approach to references. Unlike in ISO C++, expressions can actually have reference types. Naming a reference object yields an lvalue expression with reference type, rather than implicitly dereferencing the reference and giving you an lvalue to the pointed-at thing.

```cpp
#include <utility>

void f(int&);   // #1
void f(int&&);  // #2

void iso_mode(int&& r) {
  // In ISO C++, naming r is an `lvalue int` expression. 
  // #1 is called, even though r is an rvalue reference. That's weird.
  f(r);

  // In ISO C++, use std::forward or std::move to bind an rvalue reference.
  // #2 is called.
  f(std::move(r));
}

#feature on safety

void new_mode(int&& r) {
  // In the new object model, naming r is an `lvalue int&&` expression.
  // #2 is called, without using std::forward or std::move.
  f(r);
}
```

In this model, references and borrows have value semantics. There are operators to create values from places. If you want a borrow, lvalue reference or rvalue reference from an object or subobject, there's a prefix _unary-operator_ for that:

* `^x` - mutable borrow to `x`
* `^const x` - shared borrow to `x`
* `&x` - lvalue reference to `x` (convertible to pointer) 
* `&const x` - const lvalue reference to `x`
* `&&x` - rvalue reference to `x`
* `addr x` - pointer to `x`
* `addr const x` - const pointer to `x`

It would get noisy to use these everywhere, and C++ people are already used to implicit operations. There are standard conversions from lvalues to const versions of reference-like types. The effect is that **all mutations are explicit**. You don't have to wonder about side-effects. If you're passing arguments to a function, and you don't see `^`, `&` or `&&` before it, it's going to be modified by that function.

```cpp
#feature on safety
#include <iostream>

void f1(int^);
void f2(const int^);

void f3(int&);
void f4(const int&);

void f5(int&&);

int main() {
  int x = 1;

  // Explicit ^ for mut borrow required.
  f1(^x);

  // Standard conversion or explicit ^const for shared borrow.
  f2(x);
  f2(^const x);

  // Explicit & for lvalue ref required.
  f3(&x);

  // Standard conversion or explicit &const for const lvalue ref.
  f4(x);
  f4(&const x);

  // Explicit rvalue ref.
  f5(&&x);

  // Explicit & for non-const lvalue reference operands.
  &std::cout<< "Hello mutation\n";
}
```

Most people concede that Rust has the better defaults: in Rust, things are const by default; in C++, they're mutable by default. But by supporting standard conversions to const references, reference binding in Safe C++ is less noisy than the Rust equivalent, while being no less precise.

```cpp
// Rust:
f(&mut x);    // Pass by mutable borrow.
f(&x);        // Pass by shared borrow.

// C++:
f(^x);        // Pass by mutable borrow.
f(x);         // Pass by shared borrow.
f(^const x);  // Extra verbose -- call attention to it.

f(&x);        // Pass by mutable lvalue ref.
f(x);         // Pass by const lvalue ref.
f(&const x);  // Extra verbose -- call attention to it.
```

Is there a downside to rely on standard conversions for producing const references? Not really... If the parameter is pass-by-value, and your argument type is trivially copyable, then it'll copy. If the parameter is pass-by-value, and your argument type is non-trivially copyable, then the compiler will prompt for a `rel` or `cpy` token to resolve how to pass the argument. If the parameter is pass-by-const-reference, it'll bind implicitly. You're not going to accidentally hit the slow path by making use of this convenience.

## Object postfix

The _unary-operator_ syntax for mutable borrows, `^x`, will ready function arguments. How do we borrow the object for member function calls? For example, `vec.push_back(1)` is ill-formed under explicit mutation model, because `vec` is mutated, and there's no token signifying that.

Circle introduces an _object postfix_ for effecting borrow and reference binding on the object of member function calls. `vec^.push_back(1)` takes a mutable borrow on `vec` (assuming that lvalue is non-const) and calls the `push_back` member function. 

```cpp
#feature on safety

template<typename T>
struct Vec {
  size_t size(const self^) noexcept safe;
  void push_back(self^, T value) safe;
};

int main() safe {
  Vec<size_t> vec { };
  vec^.push_back(vec.size());
}
```

The inner call to `size` performs an implicit shared borrow to bind the `const self^` receiver parameter to the `vec` object. The outer call uses the object postfix `^` to mutably borrow `vec` and bind it to `self^`. Thanks to two-phase borrows,[^two-phase] the push_back argument is completely read out into a new temporary value before the mutable reference is _activated_. Without this extension to the borrow checker, the compiler would flag the implicit shared borrow on `vec` (in `vec.size()`) as invalidating the mutable borrow that was taken for the push_back.

Rust doesn't support function overloading. It considers at most one function candidate in each trait that the receiver implements, and it performs implicit relocation or shared or mutable borrows on the object argument, as required by the candidate's receiver type. Since C++ supports function overloading, users need a way to explicitly choose between candidates that may only differ in their receiver type.

* `obj^.foo()` - Mutable borrow of obj in call.
* `obj&.foo()` - Mutable lvalue reference of obj in call.
* `obj&&.foo()` - Mutable rvalue reference of obj in call.
* `obj rel.foo()` - Relocation of obj in call.
* `obj cpy.foo()` - Copy of obj in call.

```cpp
#feature on safety

struct Foo {
  // #1 Pass by shared borrow.
  void f(const self^, int) safe;

  // #2 Pass by mutable borrow.
  void f(self^, int) safe;

  // #3 Pass by value.
  void f(self, int) safe;
};

int main() safe {
  Foo obj { };

  // Pass by shared borrow - Calls #1.
  obj.f(1);

  // Pass by mutable borrow - Calls #2.
  obj^.f(2);

  // Pass by copy - Calls #3.
  obj cpy.f(3);

  // Pass by relocation - Calls #3.
  // obj is uninitialized after call.
  obj rel.f(3);
};
```

Note that Safe C++ provides _consuming member functions_, which pass their receiver types by value. To invoking consuming functions, use the `cpy` or `rel` tokens. To bind a mutable borrow or reference, use the `^`, `&` or `&&` postfix tokens. And if you want to pass by shared borrow or constant reference, you can rely on the standard conversions to cover your needs.

_Object postfix_ tokens, along with the various reference-binding operators, provide necessary precision for choosing the right overload. Shared and mutable borrows are treated unequally, and accidentally binding a mutable borrow when all you need is a shared borrow could lead to a conflict with the borrow checker.

## Choice types

### Pattern matching

[^copy-trait]: [`Copy` in `std::marker`](https://doc.rust-lang.org/std/marker/trait.Copy.html)

[^clone-trait]: [`Clone` in `std::clone`](https://doc.rust-lang.org/std/clone/trait.Clone.html)

[^drop]: [`drop` in `std::ops`](https://doc.rust-lang.org/std/ops/trait.Drop.html)

[^unique_ptr-reset]: [`std::unique_ptr::reset`](https://en.cppreference.com/w/cpp/memory/unique_ptr/reset)

[^two-phase]: [Two-phase borrows](https://rustc-dev-guide.rust-lang.org/borrow_check/two_phase_borrows.html#two-phase-borrows)