# Lifetime safety

There's one widely deployed solution to lifetime safety. That's garbage collection. In GC, the scope of an object is extended as long as there are live references to it. When there are no more live references, the system is free to destroy the object. Most memory safe languages use tracing garbage collection.[^tracing-gc] Some, like Python and Swift, use automatic reference counting,[^arc] a flavor of garbage collection with different tradeoffs.

Garbage collection requires storing objects on the _heap_. But C++ is about _manual memory management_. We need to track references to objects on the _stack_ as well as on the heap. As the stack unwinds objects are destroyed. We can't extend their duration beyond their lexical scopes. Borrow checking[^borrow-checking] is a kind of compile-time analysis that prevents using a reference after an object has gone out of scope. That is, it solves use-after-free and iterator invalidation bugs.

## Use-after-free

`std::string_view`[^string_view] was added to C++ as a safer alternatives to passing character pointers around. Unfortunately, it's so safe unsafe that its said to promote use-after-free bugs. 

```cpp
#include <iostream>
#include <string>
#include <string_view>

int main() {
  std::string s = "Hellooooooooooooooo ";
  std::string_view sv = s + "World\n";
  std::cout << sv;
}
```
```txt
$ circle string_view.cxx 
$ ./string_view
@.ooo World
```

`s` is initialized with a long string, which makes it use storage on the heap. The `string::operator+` returns a temporary `std::string` object, also with the data stored on the heap. `sv` is initialized by calling the `string::operator string_view()` conversion function on the temporary.[^string_conversion] The temporary string goes out of scope at the end of that statement, its storage is returned to the heap, and the user prints a string view with dangling pointers.

This design is full of sharp edges. It should not have made the ISO Standard. But C++ didn't have great alternatives, since it lacks borrow checking, which is the technology that flags these problems.

Safe C++ allows us to author lifetime-aware string_view types that provide memory safety. The compiler prohibits uses of dangling views.

```cpp
#feature on safety
#include "std2.h"

int main() safe {
  std2::string s = "Hellooooooooooooooo ";
  std2::string_view sv = s + "World\n";
  println(sv);
}
```
```txt
$ circle str0.cxx 
safety: str0.cxx:7:11
  println(sv); 
          ^
use of sv depends on expired loan
drop of temporary std2::basic_string<char, std2::allocator<char>> between its shared borrow and its use
loan created at str0.cxx:6:28
  std2::string_view sv = s + "World\n"; 
                           ^
```

The compiler flags the use of the dangling view, `println(sv)`. It marks the invalidating action, the drop of the temporary string. And it indicates where the loan was created, which is the conversion to `string_view` right after the string concatenation. See the [error reporting](lifetime.md#error-reporting) section for details on lifetime diagnostics.

## Iterator invalidation

## Borrow checking

### Scope and liveness

Key to one's understanding of lifetime safety is the distinction between scope and liveness. Consider lowering your function to instructions which are indexed by points. The set of points at which an object is initialized is its _scope_. In normal C++, this corresponds to its lexical scope. In Safe C++, due to relocation/destructive move, there are points in the lexical scope where the object may not be initialized, making the scope a subset of the lexical scope. 

The compiler lowers AST to MIR control flow graph and runs _initialization analysis_, a form of forward dataflow analysis[^dataflow-analysis] that computes the scope of all local variables. If a local variable has been relocated or dropped, and is then named in an expression, the scope information helps the compiler flag this as an illegal usage.

Liveness is a different property than scope, but they're often confused: users speak of "lifetime" to mean initialization or scope, while backend engineers speak of "lifetime" to mean liveness. Liveness is the set of points where the value stored in a value is yet to be used.

```cpp
void f(int);

int main() {
  int x = 1;  // x is live due to use of 1 below.
  f(x);       // not live, because 1 isn't loaded out again.

  x = 2;      // not live, because 2 isn't loaded out.
  
  x = 3;      // live because 3 is used below.
  f(x);       // still live, because 3 is used below.
  f(x);       // not live, because 3 isn't used again.
}
```

Live analysis[^live-analysis] is a reverse dataflow computation. Start at the return instruction of the control flow graph and work your way up to the entry point. When you encounter a load instruction, that variable becomes live. When you encounter a store instruction, that variable is marked dead.

The liveness property is useful in register alloction: you only care about representing a variable in register while it's holding a value that has yet to be used. But we're solving lifetime safety, we're not doing code generation. Here, we're only concerned with liveness as a property of _borrows_.

```cpp
#feature on safety
void f(int);

int main() {
  int^ ref;   // An uninitialized borrow.
  {
    int x = 1; 
    ref = ^x; // *ref is dereferenced below, so ref is live.
    f(*ref);  // now ref is dead.

    int y = 2;
    ref = ^y; // ref is live again
    f(*ref);  // ref is still live, due to read below.
  }

  f(*ref);    // ref was live but y was uninitialized.
}
```

Borrows are checked references. It's a compile-time error to use a borrow after the data it refers to has gone out of scope. Consider the set of all live references at each point in the program. Is there an invalidating action on a place referred to by one of these live references? If so, that's a contradiction that makes the program ill-formed. In this example, the contradiction occurs when `y` goes out of scope, because at that point, `ref` is a live reference to it. What makes `ref` live at that point?--The last `f(*ref)` expression in the program.

It's not enough to compute liveness of references. To determine the invalidating actions, it's important to know which place the live borrow refers to. `ref` is live until the end of the function, but `x` going out of scope is not an invalidating function, because `ref` doesn't refer to `x` anymore. We need data structures that indicate not just when a borrow is live, but to which places it may refer.

### Systems of constraints

NLL borrow checking,[^borrow-checking] is Rust's intentive method for testing invalidating actions against live borrows in the presence of control flow, re-assignments and function calls. The algorithm involves generating a system of lifetime constraints which map borrow variables back to _loans_, growing points until all the constraints are satisfied, and then testing invalidating actions against all loans in scope. 

A loan is the action that forms a reference to a place. In the example above, there are two loans: `^x` and `^y`. Solving the constraint equation extends the liveness of loans `^x` for and `^y` up until the point of the last dereferences to them. When `y` goes out of scope, it doesn't invalidate the loan `^x`, because that's not live. But it does invalidate the loan `^y`, which is lifetime extended by the final `f(*ref)` expression.

Liveness is stored in bit vectors called `regions`. There's a region for loans `^x` and `^y` and there's a region for variables with borrow types, such as `ref`. There are also regions for user-defined types with lifetime parameters, such as `string_view`.

A lifetime constraint `'R0 : 'R1 : P` reads as "region 0 outlives region 1 at point P." The compiler emits constraints when encountering assignment and function calls involving types with regions. 

```cpp
    #feature on safety
    void f(int);

    int main() {
P0:   int^ ref;          // ref is 'R0
      {
P1:     int x = 1; 
P2:     <loan R1> = ^x;  // ^x is loan 'R1
P3:     ref = <loan R1>; // 'R1 : 'R0 @ P3
P4:     f(*ref); 

P5:     int y = 2;
P6:     <loan R2> = ^y;  // ^y is loan 'R2
P7:     ref = <loan R2>; // 'R2 : 'R0 @ P7
P8:     f(*ref);

P9:     drop y
P10:    drop x
      }
    
P11:  f(*ref);
    }
```

I've relabelled the example to show function points and region names of variables and loans. If we run live analysis on 'R0, the region for the variable `ref`, we see it's live at points 'R0 = { 4, 8, 9, 10, 11 }. We'll grow the loan regions 'R1 and 'R2 until their constraint equations are satisfied.

`'R1 : 'R0 @ P3` means that starting at P3, the 'R1 contains all points 'R0 does, along all control flow paths, as long as 'R0 is live. 'R1 = { 3, 4 }. Grow 'R2 the same way: 'R2 = { 7, 8, 9, 10, 11 }.

Now we can hunt for contradictions. Visit each point in the function and consider, "is there a read, write, move, drop or other invalidating action on any of the loans in scope?" The only potential invalidating actions are the drops of `x` and `y` where they fall out of scope. At P9, the loan `^y` is in scope, because P9 is an element of its region 'R2. This is a conflicting action, because the loan is also on the variable `y`. That raises a borrow checker error. There's also a drop at P10. P10 is in the region for `^y`, but that is not an invalidating action, because the loan is not on a place that overlaps with with `x`, the operand of the drop.

The Law of Exclusivity is enforced at this point. A new mutable loan is an invalidating action on loans that are live at an overlapping place. A new shared loan is an invalidating action on mutable loans that are live at an overlapping place. Additionally, storing to variables is always an invalidating action when there is any loan, shared or mutable, on an overlapping place.

### Error reporting

The borrow checker is concerned with invalidating actions on in-scope loans. There are three instructions at play:

* **(B)** The creation of the loan. This is the lvalue-to-borrow operation, equivalent to an addressof (&). 
* **(A)** The action that invalidates the loan. That includes taking a mutable borrow on a place with a shared loan, or taking any borrow or writing to a place with a mutable borrow. These actions could lead to use-after-free bugs.
* **(U)** A use that extends the liveness of the borrow past the point of the invalidating action.

```cpp
#feature on safety
#include "std2.h"

using namespace std2;

int main() safe {
  // Make a unique pointer.
  string s = "Hello safety";

  // (B) - borrow occurs here.
  const string^ ref = s;

  // (A) - invalidating action
  drp s;

  // (U) - use that extends borrow
  println(*ref);
}
```
```txt
$ circle use1.cxx 
safety: use1.cxx:17:11
  println(*ref); 
          ^
use of *ref depends on expired loan
drop of s between its shared borrow and its use
invalidating operation at use1.cxx:14:3
  drp s; 
  ^
loan created at use1.cxx:11:23
  const string^ ref = s; 
                      ^
```

When helpful, Circle tries to identify all three of these points when forming borrow checker errors. Usually they're printed in bottom-to-top order. That is, the first source location printed is the location of the use of the invalidated loan. Next, the invalidating action is categorized and located. Lastly, the creation of the loan is indicated.

The invariants that are tested are established with a network of lifetime constraints. It might not be the case that the invalidating action is obviously related to either the place of the loan or the use that extends the loan. More completely describing the chain of constraints could users diagnose borrow checker errors. But there's a fine line between presenting an error like the one above, which is already pretty wordy, and inundating programmers with too much information.

### Calling functions

Borrow checking is really easy to understand when applied to a single function. The function is lowered to a control flow graph, the compiler assigns regions to loans and borrow variables, emits lifetime constraints where there are assignments, iteratively grows regions until the constraints are solved, and walks the instructions, checking for invalidating actions on loans in scope. The compiler automatically assigns regions to all loans and borrow variables. Within the definition of the function, there's nothing it can't analyze. The complexity arises when passing and receiving borrows through function calls.

Whole program analysis is not practical. In order to extend static lifetime safety guarantees outside of single functions, we have to introduce _lifetime contracts_ on function boundaries that are satisfied by both caller and callee. These contracts are noted by _lifetime parameters_.

```cpp
#feature on safety

auto get_x/(a, b)(const int^/a x, const int^/b y) -> const int^/a {
  return x;
}
auto get_y/(a, b)(const int^/a x, const int^/b y) -> const int^/b {
  return y;
}

int main() {
  const int^ ref1;
  const int^ ref2;
  int x = 1;
  {  
    int y = 2;
    ref1 = get_x(x, y); 
    ref2 = get_y(x, y);
  }
  int val1 = *ref1;  // OK.
  int val2 = *ref2;  // Borrow checker error.
}
```
```txt
$ circle get.cxx
safety: get.cxx:20:14
  int val2 = *ref2; 
             ^
use of *ref2 depends on expired loan
drop of y between its shared borrow and its use
y declared at get.cxx:15:9
    int y = 2; 
        ^
loan created at get.cxx:17:21
    ref2 = get_y(x, y); 
                    ^
```

Inside function declarations and function types, borrow types must be qualified with lifetime arguments. The arguments name lifetime parameters associated with the function. 

`get_x` takes two shared borrow parameters and returns a shared borrow. The return type is marked with the lifetime parameter `/a`, which corresponds with the lifetime argument on the returned value `x`. `get_y` is declared to return a shared borrow with a lifetime associated with the parameter `y`. Since we're not specifying an _outlives-constraint_ between the lifetime parameters, the function bodies can't assume anything about how these lifetimes relate to each other. It would be ill-formed for `get_x` to return `y` or `get_y` to return `x`.

The caller, `main` performs borrow checking on its side of the function calls and assumes `get_x` and `get_y` perform borrow checking on their side of the function calls. The function lifetime parameterizations define a contract so that each side can be sure that its caller or callee is upholding program soundness.

Compiling this code raises a borrow checker error when dereferencing `*ref2`, since it's a use-after-free. Static analysis on `main` knows this even without looking into the definition of `get_y`. The lifetime parameterization generates constraints at the point of the function call, so that the region on the loan on `y` outlives the region on the result object `ref2`.

```cpp
    int main() {
P0:   const int^ ref1;        // ref1 is 'R0
P1:   const int^ ref2;        // ref2 is 'R1
P2:   int x = 1;
      {  
        int y = 2;
P3:     <loan R2> = ^const x; // ^const x is 'R2
P4:     <loan R3> = ^const y; // ^const y is 'R3
P5:     ref1 = get_x(<loan R2>, <loan R3>);
        // /a is 'R4.
        // /b is 'R5.
        // 'R2 : 'R4 @ P5
        // 'R3 : 'R5 @ P5
        // 'R4 : 'R0 @ P5

P6:     <loan R6> = ^const x; // ^const x is 'R6
P7:     <loan R7> = ^const y; // ^const y is 'R7
P8:     ref2 = get_y(<loan R6>, <loan R7>);
        // /a is 'R8'.
        // /b is 'R9'.
        // 'R6 : 'R8 @ P8
        // 'R7 : 'R9 @ P8
        // 'R9 : 'R1 @ P8

P9:     drop y
      }
P10:  int val1 = *ref1;  // OK.
P11:  int val2 = *ref2;  // Borrow checker error.

P12:  drop x
    }
```

For every function calls, the lifetime parameters of that function are assigned regions. The regions of the function arguments outlive their corresponding lifetime parameters, and the lifetime parameters outlive their correspond result object parameters. This creates a chain of custody from the arguments, through the function, and out the result. The caller doesn't have to know the definition of the function, because it upholds the constraints at the point of the call, and the callee upholds the constraints in the definition of the function.

Let's solve for the regions of the four loans:

```txt
'R2 = { 4, 5, 6, 7, 8, 9, 10 }
'R3 = { 5 }
'R6 = { 7, 8, }
'R7 = { 8, 9, 10, 11 }
```

The drops of `x` and `y`, when they go out of scope, are the potentially invalidating actions. `y` goes out of scope at P9, and the loans with regions `R2` and `R7` are live at P9 (because they have 9 in their sets). The 'R2 loan borrows variable `x`, which is non-overlapping with the drop operand `y`, so it's not an invalidating action. The 'R7 loan borrows variable `y`, which is overlapping with the drop operand `y`, so we get a borrow checker error. The drop of `x` is benign, since no loan is live at P12.

### Reborrows

## Lifetime parameters

On function types and function declarators, the _lifetime-parameters-list_ goes right after the _declarator-id_.

```cpp
// A function type declarator.
using F1 = void/(a, b)(int^/a, int^/b) safe;

// A function declaration with the same type
void f1/(a, b)(int^/a, int^/b) safe;
```

Borrows are first-class _lifetime binders_. You can also declare class and class templates that bind lifetimes by putting _lifetime-parameters-lists_ after their _declarator-ids_.

```cpp
template<typename char_type>
class basic_string_view/(a);
```

All types with lifetime binders named in function declarations and data members must be qualified with lifetime arguments. This helps define the soundness contract between callers and callees.

Often, it's necessary to specify requirements between lifetime parameters. These take the form of _outlives-constraints_. They're specified in a _where-clause_ inside the _lifetime-parameter-list_.

```cpp
using F2 = void/(a, b where a : b)(int^/a x, int^/b y) safe;
```

Rust chose a different syntax, mixing lifetime parameters and type parameters into the same parameter list. Coming from C++, where templates supply our generics, I find this misleading. Lifetime parameters are really nothing like template parameters. A function with lifetime parameters isn't really a generic function. Lifetime parameters are never substituted with any kind of concrete lifetime argument. Instead, the relationship between lifetime parameters, as deduced from _outlives-constraints_, implied bounds and variances of function parameters, establishes constraints that are used by the borrow checker at the point of call.

```rust
// A Rust declaration.
fn f<'a, 'b, T>(x: &'a mut T, y: &'b T) where 'a : 'b { }
```
```cpp
// A C++ declaration.
template<typename T>
void f/(a, b where a : b)(T^/a x, const T^/b y) { }
```

Rust uses single-quotes to introduce lifetime parameters and lifetime arguments. That wasn't a realistic choice for me, because C supports multi-character literals.[^character-literal] This cursed feature, in which literals like `'abcd'` evaluate to constants of type `int`, makes lexing Rust-style lifetime arguments very messy.

```cpp
template<typename T>
void f/(a, b where a : b)(T^/a x, const T^/b y) { }

int main() {
  int x = 1;
  int y = 2;
  f(^x, y);
}
```
```cpp
    int main() {
P0:   int x = 1;
P1:   int y = 2;
P2:   <loan R0> = ^x;
P3:   <loan R1> = ^const y;
P4:   f(<loan R0>, <loan R1>);
      // /a is 'R2
      // /b is 'R3
      // 'R2 : 'R3 @ P4 - `where a : b`
      // 'R0 : 'R2 @ P4
      // 'R1 : 'R3 @ P4
    }
```

The _where-clause_ establishes the relationship that `/a` outlives `/b`. Does this mean that the variable pointed at by `x` goes out of scope later than the variable pointed at by `y`? No, that would be too straight-forward. This declaration emits a _lifetime-constraint_ at the point of the function call. The regions of the arguments already constrain the regions of the lifetime parameters. The _where-clause_ constrains the lifetime parameters to one another. `f`'s outlives constraint is responsible for the constraint `'R2 : 'R3 @ P4`.

Lifetime parameters and _where-clauses_ are a facility for instructing the borrow checker. The obvious mental model is that the lifetimes of references are connected to the scope of the objects they point to. But this is not accurate. Think about lifetimes as defining rules that can't be violated, with the borrow checker looking for contradictions of these rules.

### Free regions

### Lifetime canonicalization

```cpp
#feature on safety

// Two distinct lifetimes with no constraint.
using F1 = void/(a, b)(int^/a, int^/b) safe;

// These are the same.
using F2 = void/(a, b where a : b)(int^/a, int^/b) safe;
using F3 = void/(a, b where b : a)(int^/b, int^/a) safe;
static_assert(F2 == F3);

// They differ from F1, due to the outlives-constraint.
static_assert(F1 != F2);

// These are the same. 
using F4 = void/(a, b where a : b, b : a)(int^/a, int^/b) safe;
using F5 = void/(a)                      (int^/a, int^/a) safe;
static_assert(F4 == F5);

// They differ from F2, due to the constraint going both directions.
static_assert(F2 != F4);
```

Lifetime parameterizations are part of the function's type. But different textual parameterizations may still result in the same type! `F1` and `F2` have different parameterizations and are different types. But `F2` and `F3` have different parameterizations yet are the same type. Likewise, `F4` and `F5` are the same type, even though `F4` has two lifetime parameters and two outlives constraints.

The compiler maps all non-dependent types to canonical types. When comparing types for equality, it compares the pointers to their canonical types. This is necessary to support typedefs and alias templates that appear in functions--we need to strip away those inessential details and get to the canonical types within. The lifetime parameterizations the user writes also map to canonical parameterizations.

Think about lifetime parameterizations as a directed graph. Lifetime parameters are the nodes and outlives constraints define the edges. The compiler finds the strongly connected components[^scc] of this graph. That is, it identifies all cycles and reduces them into SCC nodes. In `F4`, the `/a` and `/b` lifetime parameters constrain one another, and there are collapsed into a strongly connected component. The canonical function type is encoded using SCCs as lifetime parameters. Both `F4` and `F5` map to the same canonical type, and therefore compare the same.

During the type relation pass that generates lifetime constraints for function calls in the MIR, arguments and result object regions are constrained to regions of the canonical type's SCCs, rather than the lifetime parameters of the declared type. This reduces the number of regions the borrow checker solves for. But the big reason for this process is to permit writing compatible functions even in the face lifetime normalization.  

### Lifetimes and templates



### Lifetime normalization

TODO:

[^tracing-gc]: [Tracing garbage collection](https://en.wikipedia.org/wiki/Tracing_garbage_collection)

[^arc]: [Automatic reference counting](https://docs.swift.org/swift-book/documentation/the-swift-programming-language/automaticreferencecounting/)

[^borrow-checking]: [The Rust RFC Book - Non-lexical lifetimes](https://rust-lang.github.io/rfcs/2094-nll.html)

[^string_view]: [std::basic_string_view](https://en.cppreference.com/w/cpp/string/basic_string_view)

[^string_conversion]: [std::string::operator string_view](https://en.cppreference.com/w/cpp/string/basic_string/operator_basic_string_view)

[^dataflow-analysis]: [Data-flow analysis](https://en.wikipedia.org/wiki/Data-flow_analysis)

[^live-analysis]: [Live-variable analysis](https://en.wikipedia.org/wiki/Live-variable_analysis)

[^character-literal]: [Character literal](https://en.cppreference.com/w/cpp/language/character_literal)

[^scc]: [Strongly connected component](https://en.wikipedia.org/wiki/Strongly_connected_component)