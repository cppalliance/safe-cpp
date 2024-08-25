# Safe C++

## The call for memory safety

Over the past two years, the United States Government has been issuing warnings about memory-unsafe programming languages with increasing urgency. Much of the country's critical infrastructure relies on software written in C and C++, languages which are very memory _unsafe_, leaving these systems more vulnerable to exploits by adversaries.

* Nov. 10, 2022 - **NSA Releases Guidance on How to Protect Against Software Memory Safety Issues**[^nsa-guidance]

* Sep. 20, 2023 - **The Urgent Need for Memory Safety in Software Products**[^cisa-urgent]

* Dec. 6, 2023 - **CISA Releases Joint Guide for Software Manufacturers: The Case for Memory Safe Roadmaps**[^cisa-roadmaps]

* Feb. 26, 2024 - **Future Software Should Be Memory Safe**[^white-house]

* May 7, 2024 - **National Cybersecurity Strategy Implementation Plan**[^ncsi-plan]

The government papers are backed by industry research. Microsoft's bug telemetry reveals that 70% of its vulnerabilities would be stopped by memory-safe programming languages.[^ms-vulnerabilities] Google's research has found 68% of 0day exploits are related to memory corruption.[^google-0day] 

* Mar. 4, 2024 - **Secure by Design: Google's Perspective on Memory Safety**[^secure-by-design]

Security professionals loudly advocate that projects migrate away from C++ and start using memory safe languages. But the scale of the problem is daunting. C++ powers products that have generated trillions of dollars of value. There are a lot of C++ programmers and a lot of C++ code. Given how wide-spread C and C++ code is, what can industry really do to improve software quality and reduce vulnerabilities? What are the options for introducing new memory safe code into existing projects and hardening software that already exists?

There's only one systems-level/non-garbage collected language that provides rigorous memory safety. That's the Rust language.[^rust-language] But while they play in the same space, C++ and Rust are idiomatically very different with limited interop capability, making incremental migration from C++ to Rust a slow, painstaking process.

Rust lacks function overloading, templates and inheritance. C++ lacks traits, relocation and lifetime parameters. These discrepancies are responsible for an impedence mismatch when interfacing the two languages. Most code generators for inter-language bindings do not even attempt to represent the features of one language in terms of the features of another. They typically identify a number of special vocabulary types,[^vocabulary-types] which have first-class ergonomics, and limit functionality of other constructs.

The foreignness of Rust for career C++ developers along with the inadequacies of interop tools makes hardening C++ applications by rewriting critical sections in Rust very difficult. Why is there no in-language solution to memory safety? _Why not a Safe C++?_

[^nsa-guidance]: [NSA Releases Guidance on How to Protect Against Software Memory Safety Issues](https://www.nsa.gov/Press-Room/News-Highlights/Article/Article/3215760/nsa-releases-guidance-on-how-to-protect-against-software-memory-safety-issues/)

[^cisa-urgent]: [The Urgent Need for Memory Safety in Software Products](https://www.cisa.gov/news-events/news/urgent-need-memory-safety-software-products)

[^cisa-roadmaps]: [CISA Releases Joint Guide for Software Manufacturers: The Case for Memory Safe Roadmaps](https://www.cisa.gov/news-events/alerts/2023/12/06/cisa-releases-joint-guide-software-manufacturers-case-memory-safe-roadmaps)

[^white-house]: [Future Software Should Be Memory Safe](https://www.whitehouse.gov/oncd/briefing-room/2024/02/26/press-release-technical-report/)

[^ncsi-plan]: [National Cybersecurity Strategy Implementation Plan](https://www.whitehouse.gov/wp-content/uploads/2024/05/NCSIP-Version-2-FINAL-May-2024.pdf)

[^ms-vulnerabilities]: [We need a safer systems programming language](https://msrc.microsoft.com/blog/2019/07/we-need-a-safer-systems-programming-language)

[^google-0day]: [0day "In the Wild"](https://googleprojectzero.blogspot.com/p/0day.html)

[^secure-by-design]: [Secure by Design: Google's Perspective on Memory Safety](https://research.google/pubs/secure-by-design-googles-perspective-on-memory-safety/)

[^rust-language]: [The Rust Programming Language](https://doc.rust-lang.org/book/)

[^vocabulary-types]: [CXX — safe interop between Rust and C++](https://cxx.rs/bindings.html)

### Extend C++ for safety

The goal of the authors is to define a superset of C++ with a _rigorously safe subset_. Begin a new project, or take an existing one, and start writing safe code in C++. Code in the safe context exhibits the same strong safety guarantees as safe code written in Rust. 

The Safe C++ frontend (the user-facing part of the language) is a straight-forward extension of C++. It adds new types like choices and borrows, new control flow like pattern matches, and new language entities like lifetime parameters. But the Safe C++ backend (the hardware-facing part of the language) is all new technology. Safe C++ adopts that Rust's _ownership and borrowing_ approach to memory safety. Type safety is delivered with linear types. Lifetime safety is enforced with borrow checking.[^borrow-checking]. This technology works on mid-level IR,[^mir] the typed control-flow graph that is generated after the phases of translation normally responsible for C++ type safety. 

The argument for Rust: it's a clean new language designed from the ground up for safety.

The argument for Safe C++: it provides the same rigorous safety guarantees as Rust, but since it extends C++, it has unbeatable interoperability with existing code.

The goal is to write robust, sound software. Rust is a proven tool to achieve that. Safe C++ could be another viable tool. What's not viable is to continue investing in unsafe, vulnerability-ridden code.

### Properties of Safe C++

* A superset of C++ with a _safe subset_. Undefined behavior is prohibited from originating in the safe subset.
* The safe and unsafe parts of the language are clearly delineated, and users must explicitly leave the safe context to use unsafe operations.
* The safe subset must remain _useful_. If we get rid of a crucial unsafe technology, like unions or pointers, we should supply a safe alternative, like choice types or borrows. A perfectly safe language is not useful if it's so inexpressive you can't get your work done.
* The new system can't break existing code. If you point a Safe C++ compiler at existing C++ code, that code must compile normally. Users opt into the new safety mechanisms. Safe C++ is an extension of C++. It's not a new language.

```cpp
#feature on safety
#include <std2/vector.h>
#include <cstdio>

int main() safe {
  std2::vector<int> vec { 11, 15, 20 };

  for(int x : vec) {
    // Ill-formed. mutate of vec invalidates iterator in ranged-for.
    if(x % 2)
      mut vec.push_back(x);

    unsafe { printf("%d\n", x); }
  }
}
```
```txt
SHOW OUTPUT
```

Consider this demonstration of Safe C++ that catches iterator invalidation, which would lead to a use-after-free bug. Let's break it down line by line:

Line 1: `#feature on safety` - Turn on the new safety-related keywords within this file. Other files in your translation unit are unaffected. This is how Safe C++ avoids breaking existing code--everything is opt-in, including the new keywords and syntax. The safety feature changes the object model for function definitions, enabling object relocation, partial and deferred initialization. It lowers function definitions to mid-level intermediate representation (MIR)[^mir], on which borrow checking is performed to flag potential use-after-free bugs on checked references.

Line 2: `#include <std2/vector.h>` - Include the new safe containers and algorithms. Safety hardening is about reducing your exposure to unsafe APIs. The current Standard Library is full of unsafe APIs. The new Standard Library in namespace `std2` will provide the same basic functionality, but with containers that are lifetime-aware and type safe.

Line 4: `int main() safe` - The new _safe-specifier_ is part of a function's type, just like _noexcept-specifier_. To callers, the function is marked as safe, so that it can be called from a safe context. `main`'s definition starts in a safe context, so unsafe operations like pointer dereferences, and calling unsafe functions, is not allowed. Rust's functions are safe by default. C++'s are unsafe by default. But that's now just a syntax difference. Once you enter a safe context in C++ by using the _safe-specifier_, you're backed by the same rigorous safety guarantees that Rust provides.

Line 5: `std2::vector<int> vec { 11, 15, 20 };` - List initialization of a memory-safe vector. This vector is aware of lifetime parameters, so borrow checking would extend to element types that have lifetimes. The vector's constructor doesn't use `std::initializer_list<int>`[^init-list]. That type is problematic for two reasons: first, users are given pointers into the argument data, and reading from pointers is unsafe; second, the `std::initializer_list` _doesn't own_ its data, making relocation impossible. For these reasons, Safe C++ introduces a `std2::initializer_list<T>`, which can be used in a safe context and supports our ownership object model.

Line 7: `for(int x : vec)` - Ranged-for on the vector. The standard mechanism[^ranged-for] returns a pair of iterators, which are pointers wrapped in classes. C++ iterators are unsafe. They come in begin and end pairs, and don't share common lifetime parameters, making borrow checking them impractical. The Safe C++ version uses slice iterators, which resemble Rust's `Iterator`.[^rust-iterator] These safe types use lifetime parameters making them robust against iterator invalidation.

Line 10: `mut vec.push_back(x);` - Push a value onto the vector. What's the `mut` doing there? That token establishes a _mutable context_, which permits enables standard conversions from lvalues to mutable borrows and references. When `#feature on safety` is enabled, _all mutations are explicit_. Explicit mutation lends precision when choosing between shared borrows and mutable borrows of an object. Rust doesn't feature function overloading, so it will implicitly borrow (mutably or shared) from the member function's object. C++ of course has function overloading, so we'll need to be explicit in order to get the overload we want.

Line 12: `unsafe { printf("%d\n", x); }` - Call `printf`. It's a very unsafe function. Since we're in a safe context, we have to escape with the `unsafe` keyword. Safe C++ doesn't lock off any parts of the C++ language. You're free to shoot yourself in the foot, provided you sign the waiver in the form of the `unsafe` keyword. `unsafe` means that you swear to follow the preconditions of the function, rather than relying on the compiler to ensure those preconditions for you.

If `main` checks out syntatically, its AST is lowered to MIR, where it is borrow checked. The hidden iterator that powers the ranged-for loop stays initialized during execution of the loop. The `push_back` _invalidates_ that iterator, by mutating a place (the vector) that the iterator has a constraint on. When the value `x` is next loaded out of the iterator, the borrow checker raises an error: `mutable borrow of vec between its shared borrow and its use`. The borrow checker prevents Safe C++ from compiling a program that may have exhibited undefined behavior. This is all done at compile time, with no impact on your program's size or speed.

This sample is only a few lines, but it introduces several new mechanisms and types. Security experts keep reminding us that **C++ is very unsafe**. It takes a systematic effort to supply a superset of the language with a safe subset that has enough flexibility to remain expressive.

[^borrow-checking]: [The Rust RFC Book - Non-lexical lifetimes](https://rust-lang.github.io/rfcs/2094-nll.html)

[^mir]: [The Rust RFC Book - Mid-level IR](https://rust-lang.github.io/rfcs/1211-mir.html)

[^init-list]: [std::initializer_list](https://en.cppreference.com/w/cpp/utility/initializer_list)

[^ranged-for]: [Range-based for loop](https://en.cppreference.com/w/cpp/language/range-for)

[^rust-iterator]: [`Iterator` in `std::iter`](https://doc.rust-lang.org/std/iter/trait.Iterator.html)

### Memory safety as terms and conditions

Memory-safe languages are predicated on a basic observation of human nature: people would rather try something, and only then ask for help if it doesn't work. For programming, this means developers try to use a library, and only then read the docs if they can't get it to work. This has proven very dangerous, since appearing to work is not the same as working.

Many C++ functions have preconditions that are only known after careful of their perusal of their documentation. Preconditions can be anything; users don't come with expectations as to what safe usage should look like. Violating preconditions, which is possible with benign-looking usage, causes undefined behavior and opens your software to attack. **Software safety and security should not be predicated on programmers following documentation.** 

Here's the value proposition: compiler and library vendors make an extra effort to provide a robust environment so that users _don't have to read the docs_. No matter how they use the language and library, their actions will not raise undefined behavior and open the software to safety-related exploits. No system can guard against all misuse, and hastily written code may have plenty of logic bugs. But those logic bugs won't lead to memory safety vulnerabilities.

Consider an old libc function, `std::isprint`,[^isprint] that exhibits unsafe design. This function takes an `int` parameter. _But it's not valid to call `std::isprint` for all int arguments_. The preconditions state the function be called only with arguments between -1 and 255:

> Like all other functions from `<cctype>`, the behavior of `std::isprint` is undefined if the argument's value is neither representable as unsigned char nor equal to EOF. To use these functions safely with plain chars (or signed chars), the argument should first be converted to unsigned char.
> Similarly, they should not be directly used with standard algorithms when the iterator's value type is char or signed char. Instead, convert the value to unsigned char first.

It seems natural, in the year 2024, to pass UNICODE code points to functions that are typed with `int` and deal with characters. While the mistake is the caller's for not reading the documentation and following the preconditions, this design goes against human nature. Do not rely on the programmer to closely read the docs before using your function. The safe context provided by memory safe languages prevents usage or authoring of functions like `std::isprint` which exhibit undefined behavior when called with invalid arguments.

Rust's approach to safety[^safe-unsafe-meaning] is about defining responsibility for enforcing preconditions. In a safe context, the user can call safe functions without compromising program soundness. Failure to read the docs may risk correctness, but it won't risk undefined behavior. When the user wants to call an unsafe function from a safe context, they _explicitly assume responsibility_ for sound usage of that unsafe function. The user writes the `unsafe` token as a kind of contract: the user has read the terms and conditions of the unsafe function and affirms that it's not being called in a way that violates its preconditions.

Who is to blame when undefined behavior is detected, the caller or the callee? ISO C++ does not have an answer, making it an unreliable language. But Rust's safety model does: whoever typed out the `unsafe` token is at fault. Safe C++ adopts the same principle. Code is divided into unsafe and safe contexts. Unsafe operations may only occur in unsafe contexts. Dropping from a safe context to an unsafe context requires use of the `unsafe` keyword. This leaves an artifact that makes for easy audits: reviewers search for the `unsafe` keyword and focus their attention there first.

Consider the design of an `std2::isprint` function. If it's marked `safe`, it must be sound for all integer arguments. If it's called with an argument that is out of its supported range, it must fail in a deterministic way: it return an error code, it could throw an exception or it could panic and abort. The designer of the function doesn't have to think very hard about soundness, they just have to follow the rules. Inside the `std2::isprint` implementation, there's probably a lookup table with capabilities for each supported character. If the lookup table is accessed with a slice, an out-of-bounds access will implicitly generate a bounds check and panic and abort on failure. If the lookup table is accessed through a pointer, the implementer writes the `unsafe` keyword, drops to the unsafe context and subscripts the pointer. The `unsafe` keyword is the programmer's oath that the subsequent unsafe operations are correct. It's obligatory for the library to test the integer argument against the range of the array and error if it's out-of-bounds before subscripting the pointer.

In ISO C++, soundness holes often occur because caller and callee don't agree on who should enforce preconditions, so neither of them do. In Safe C++, there's a convention policed by the compiler, eliminating this confusion and improving software quality.

[^isprint]: [`std::isprint`](https://en.cppreference.com/w/cpp/string/byte/isprint)

[^safe-unsafe-meaning]: [How Safe and Unsafe Interact](https://doc.rust-lang.org/nomicon/safe-unsafe-meaning.html)

## Categories of safety

It's instructive the memory safety problem down into five categories. Each of these is addressed with a different strategy.

### 1. Lifetime safety

How do we ensure that dangling references are never used? There are two safe technologies: garbage collection and borrow checking. But garbage collection moves allocations to the heap, making it incompatible with manual memory manegement. It extends object lifetimes as long as there are live references to them, making it incompatible with C++'s RAII[^raii] object model.

Borrow checker technology is a compile-time local analysis that defines a network of constraints against a function's control flow graph. Solving the constraints grows regions, which define the points at which each loan is in scope. Invalidating actions, such as reads or writes to places that overlap in-scope loans raise borrow checker errors. This is a brilliant system. It's compatible with manual memory management and RAII.

Borrow checking a function only has to consider the body of that function. It avoids whole-program analysis by instituting the _Law of Exclusivity_. Checked references (borrows) come in two flavors: mutable and shared, noted respectively as `T^` and `const T^`. There can be one live mutable reference to a place, or any number of shared references to a place, but not both at once. Upholding this principle makes it much easier to reason about your program. If a function is passed a mutable reference and some shared references, you can be certain that the function won't have side effects that, through the mutable reference, cause the invalidation of those shared references. Why not? Because you can't form shared references to an overlapping same place as the mutable reference to even pass to that function.

This principle eliminates much subtlety needed for library work. The downside is a loss of flexibility. There are idioms that are expressible with unsafe that aren't expressible with borrows. Clever engineers have spent a lot of effort developing safe analogs to existing unsafe paradigms.

Is there a learning curve? Is there disruption? Of course. But what alternative do we have? C++'s current safety story is unacceptable. If you're going to keep writing code that runs on devices attached to the network, you're going to have to harden it against vulnerabilities. If you want to use manual memory management and RAII, borrow checking is the only viable option.

[^dangling-pointer]: [Dangling pointer](https://en.wikipedia.org/wiki/Dangling_pointer)

[^raii]: [Resource acquisition is initialization](https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization)

### 2. Type safety - null pointer variety

> I call it my billion-dollar mistake. It was the invention of the null reference in 1965. At that time, I was designing the first comprehensive type system for references in an object oriented language (ALGOL W). My goal was to ensure that all use of references should be absolutely safe, with checking performed automatically by the compiler. But I couldn't resist the temptation to put in a null reference, simply because it was so easy to implement. This has led to innumerable errors, vulnerabilities, and system crashes, which have probably caused a billion dollars of pain and damage in the last forty years.
>
> -- <cite>Tony Hoare</cite>[^hoare]

The "billion-dollar mistake" is a type safety problem. Consider `std::unique_ptr`. It has two states: engaged and disengaged. The class presents member functions like `operator*` and `operator->` that are valid when the object is in the engaged state and _undefined_ when the object is disengaged. `->` is the most important API for smart pointers. Calling it when the pointer is null? That's your billion-dollar mistake.

As Hoare observes, the problem was conflating two different things, a pointer to an object and an empty state, into the same type and giving them the same interface. Smart pointers should only hold valid pointers. If you want to represent an empty state, use some other mechanism that has its own interface. Denying the null state eliminates undefined behavior.

`std2::unique_ptr` has no null state. There's no default constructor. If the object is in scope, you can dereference it without risk of undefined behavior. Why doesn't C++ simply introduce its own fixed `unique_ptr` without a null state? Blame C++11 move semantics.

How do you move objects around in C++? Use `std::move` to select the move constructor. That moves data out of the old object, leaving it in a default state. For smart pointers, that's the null state. If `unique_ptr` didn't have a null state, it couldn't be moved in C++.

Addressing the null type safety problem means entails overhauling the object model. Safe C++ features a new kind of move: [_relocation_](type.md#relocation-object-model), also called _destructive move_. Unless explicitly initialized, objects start out _uninitialized_. They can't be used in this state. When you assign to an object, it becomes initialized. When you relocate from an object, it's back to being uninitialized. If you relocate from an object inside control flow, it becomes _potentially uninitialized_, and its destructor is conditionally executed after reading an automatically-generated drop flag.

```cpp
int main() {
  // p is uninitialized.
  std2::box<int> p;

  // Error: p is uninitialized.
  int x = *p;

  // p is definitely initialized.
  p = std2::box<int>::make(5);

  // Ok.
  int y = *p;

  // p is moved into q. Now p is uninitialized again.
  auto q = rel p;

  // Error: p is uninitialized.
  int z = *p;
}
```

The _rel-expression_ names a local variable object or subobject and relocates that into a new value. The old object becomes uninitialized. Any uses of uninitialized objects generates a compiler error. Using a null `unique_ptr` was undefined behavior. Using an uninitialized one is a compile-time error.

We have to reimagine our standard library in the presence of relocation. Most kinds of resource handles include null states. These should all be replaced by safe versions to reduce exposure to unsafe APIs.

### 3. Type safety - union variety

The compiler can only relocate local variables. How do we move objects that live on the heap, or for which we only have a pointer or reference? We need to use optional types.

```cpp
template<typename T>
choice optional {
  default none,
  some(T)
};

struct Data {
  // May be engaged (some) or disengaged (none).
  optional<std2::unique_ptr<int>> value;
};
```

We use an optional type to represent both aspects of our the smart pointer: disengaged (`none`) and engaged (`some`). If you want to move the pointer, detach it from the optional, which changes the state of the optional from `some` to `none`. The C++ Standard Library has an optional type,[^optional] but it's not safe to use. The optional API is full of undefined behaviors.[^optional-undefined]

![std::optional::operator*](optional-undefined.png)

A similar class, `std::expected`, which is new to C++23, is also full of undefined behaviors.[^expected-undefined]

If we were to wrap the safe `std2::unique_ptr` in an `std::optional`, it would be just as unsafe as using `std::unique_ptr`. Using `->` with a disengaged value would cause undefined behavior.

The new `std2::optional` is a _choice type_, a first-class discriminated union, that can only be accessed with _pattern matching_. Pattern matching makes the union variety of type safety violations impossible: we can't access the wrong state of the sum type.

```cpp
#include <string>
#include <iostream>

union Value {
  int i32;
  float f32;
  double f64;
  const char* str;
};

void print(Value val) {
  // C++ does not protect us from accessing bits with the wrong type.
  // Catastrophe!
  std::cout<< val.str<< "\n";
}

int main() {
  Value value { };
  value.i32 = 101;
  print(value);
}
```

C++'s sum type support is built on top of unions. Unions are extremely unsafe. Naming a union field is like implicitly using `reintepret_cast` to convert the object's bits into the type of the field. The defects in `std::optional` and `std::expected` are of this nature: the libraries don't guard against access using an invalid type. C++ builds abstractions on top of unions, but they're not _safe_ abstractions.

```cpp
#feature on safety
#include <iostream>
#include <std2/string>

// A discriminated union that's impossible to misuse.
choice Value {
  i32(int),
  f32(float),
  f64(double),
  str(std2::string)
};

void print(Value val) safe {
  match(val) {
    // Type safety bugs are impossible inside the pattern match.
    // The alternatives listed must be exhaustive.
    .i32(i32) => unsafe { mut std::cout<< i32<< "\n" };
    .f32(f32) => unsafe { mut std::cout<< f32<< "\n" };
    .f64(f64) => unsafe { mut std::cout<< f64<< "\n" };
    .str(str) => unsafe { mut std::cout<< str<< "\n" };
  };
}

int main() safe {
  print(.i32(5));
  print(.f32(101.3f));
  print(.f64(3.15159));
  print(.str("Hello safety"));
}
```

Choice types are Safe C++'s type-safe offering. They're just like Rust's enums,[^rust-enum] one of features most credited for that language's enviable ergonomics. Accessing members of a choice object requires testing for the active type with a _match-expression_. If the match succeeds, a new declaration is bound to the corresponding payload, and that declaration is visible in the scope following the `=>`.

The compiler also performs exhaustiveness testing. Users must name all the alternatives, or use a wildcard `_` to default the unnamed ones.

Pattern matching and choice types aren't just a qualify-of-life improvement. They're a critical part of the memory safety puzzle and all modern languages provide them.

### 4. Thread safety

A memory-safe language should be robust against data races to shared mutable state. If one thread is writing to shared state, no other thread should be allowed access to it. Rust provides thread safety using a really novel extension of the type system. 

TODO

### 5. Runtime checks

TODO







## The `safe` context

Operations in the safe context are guaranteed not to cause undefined behavior. This protection is enforced with a number of methods. Some operations linked to undefined behavior can't be vetted by the frontend, during MIR analysis or with panics at runtime. Attempting to use them in the safe context makes the program ill-formed. These operations are:

* Dereference of pointers and legacy references. This may result in use-after-free undefined behaviors. Prefer using borrows, which exhibit lifetime safety thanks to the brorow checker.
* Pointer offsets. Advancing a pointer past the end or the beginning of its allocation is undefined behavior. Prefer using slices, which include bounds information.
* Pointer difference. Taking the difference of pointers into different allocations is undefined behavior.
* Pointer relational operators <, <=, > and >=. Comparing pointers into different allocations is undefined behavior.
* Accessing fields of unions. Legacy unions present a potential type safety hazard. Prefer using choice types.
* Accessing non-const objects with static storage duration. This is a data race hazard, as different users may be writing to and reading from the same memory simultaneously. This is even a hazard with thread_local storage, as the law of exclusivity cannot be guaranteed within a single thread.
* Inline ASM. The compiler generally isn't equipped to determine if inline ASM is safe, so its usage in the safe context is banned.
* Calling unsafe functions. This is banned because the unsafe function may involve any of the above operations.

Some operations are banned even in unsafe contexts. The compiler lowers function definitions to mid-level IR (MIR) and performs initialization analysis and borrow checking. These are data flow analyses, and they may expose problems with your code. These issues make the program ill-formed, regardless of the safe context:

* Use of uninitialized, partially initialized or potentially initialized objects is ill-formed. This is checked by initialization analysis.
* A conflicting action on an overlapping place with an in-scope loan is a borrow checker error. This reports potential use-after-free bugs. The law of exclusivity is enforced as part of this check.
* Free region errors. The borrow checker must confirm that lifetimes on function parameters do not outlive the constraints defined on the function's declaration. This ensures that the caller and callee agree on the lifetimes of arguments and result objects. It permits inter-procedural live analysis without attempting very expensive whole-program analysis.

Some operations are potentially unsound, but can be checked at runtime. They are checked for soundness, and if they fail, the program panics and aborts. There are there cases of this:

* Integer division by 0.
* Integer division of INT_MIN by -1.
* Out of bounds subscripting an array or slice.

### _safe-specifier_

Similar to the _noexcept-specifier_,[^noexcept-spec] function types and declarations may be marked with a _safe-specifier_. Place this after the _noexcept-specifier_. Types and functions without the _noexcept-specifer_ are assumed to be potentially throwing. Similarly, types and functions without the _safe-specifier_ are assumed to be unsafe.

```cpp
// `safe` is part of the function type.
using F1 = void(int);
using F2 = void(int) safe;
using F3 = void(int) noexcept;
using F4 = void(int) noexcept safe;
```

As with `noexcept`, `safe` is part of the function's type, so types with different _safe-specifiers_ always compare differently. 


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

Declaring functions with value-dependent _safe-specifiers_ is supported. This follows dependent _noexcept-specifiers_, allowing users to use the _safe-operator_ to query the safeness of an expression and reflect that in the _safe-specifier_.

```cpp
template<bool IsSafe>
struct foo_t {
  // Value-dependent safe-specifier. Set to the template parameter.
  void f1() safe(IsSafe);

  // Set f2's safe specifier to the safeness of arg.func().
  template<typename T>
  void f2(T arg) safe(safe(arg.func()));
};
```

### _safe-operator_

You can query the safeness of an expression in an unevaluated context with the _safe-operator_. It's analagous to the existing _noexcept-operator_.[^noexcept-operator] This is very useful when paired with _requires-clause_,[^requires-clause] as it lets you constrain inputs based on the safeness of a callable.

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

These kind of constraints are idiomatic in C++ but not supported in Rust, because that uses early-checked traits to implement generics.

[^noexcept-operator]: [Noexcept operator](https://en.cppreference.com/w/cpp/language/noexcept)

[^requires-clause]: [Requires clauses](https://en.cppreference.com/w/cpp/language/constraints#Requires_clauses)

### _unsafe-block_

The `unsafe` token is required to escape the safe context and perform operations whose soundness cannot be checked by the toolchain. The most basic unsafe escape is the _unsafe-block_. At the statement level, write `unsafe { }` and put the unsafe operations inside the braces. This does _not_ open a new lexical scope, which is different from Rust's behavior.

```cpp
SHOW unsafe { }
```

### The `unsafe` type qualifier

The Rust ecosystem was built from the bottom-up prioritizing safe code. Consequently, there's so little unsafe code that the _unsafe-block_ is generally sufficient for interfacing with it. By contrast, there are many billions of lines of unsafe C++ in the wild. The _unsafe-block_ isn't powerful enough to interface our safe and unsafe assets, as we'd be writing _unsafe-blocks_ everywhere, making a noisy mess. Worse, we'd be unable to use unsafe types from safe function templates, since the template definition wouldn't know it was dealing with unsafe template parameters. Because of the ecosystem difference, Rust does not provide guidance for this problem, and we're left to our own devices.

**Safe C++'s answer to safe/unsafe interoperability is to make safeness part of the type system.**

C++ has `const` and `volatile` type qualifiers. C++ compilers also support the `_Atomic` type qualifier,[^atomic-types] through C11. Safe C++ adds the `unsafe` type qualifier. Declare an object or data member with the `unsafe` qualifier and use it freely _even in safe contexts_. The `unsafe` token means the same thing here as it does with _unsafe-blocks_: the programmer is declaring responsibility for upholding the conditions of the object. Blame lies with the `unsafe` wielder.

```cpp
unsafe-qualifier examples
```

Naming an `unsafe` object yields an lvalue expression of the unsafe type. Calling unsafe member functions on expressions with `unsafe` types is permitted in the unsafe context. In fact, calling unsafe functions or performing unsafe operations on unsafe-qualified types is "safe" for the purpose of _safe-operator_.

```cpp

```

### unsafe qualifier and templates

Show std2::vector<std::string> usage.

[^atomic-types]: [_Atomic types](https://en.cppreference.com/w/c/language/atomic)

### _using-unsafe-declaration_

## Lifetime safety

There's one widely deployed solution to lifetime safety: garbage collection. In GC, the scope of an object is extended as long as there are live references to it. When there are no more live references, the system is free to destroy the object. Most memory safe languages use tracing garbage collection.[^tracing-gc] Some, like Python and Swift, use automatic reference counting,[^arc] a flavor of garbage collection with different tradeoffs.

Garbage collection requires storing objects on the _heap_. But C++ is about _manual memory management_. We need to track references to objects on the _stack_ as well as on the heap. As the stack unwinds objects are destroyed. We can't extend their duration beyond their lexical scopes. Borrow checking[^borrow-checking] is a kind of compile-time analysis that prevents using a reference after an object has gone out of scope. That is, it solves use-after-free and iterator invalidation bugs.

### Use-after-free

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

### Iterator invalidation

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

### Lifetime error reporting

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

### Lifetime constraints on called functinos

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

### Lifetime parameters

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

[^tracing-gc]: [Tracing garbage collection](https://en.wikipedia.org/wiki/Tracing_garbage_collection)

[^arc]: [Automatic reference counting](https://docs.swift.org/swift-book/documentation/the-swift-programming-language/automaticreferencecounting/)

[^borrow-checking]: [The Rust RFC Book - Non-lexical lifetimes](https://rust-lang.github.io/rfcs/2094-nll.html)

[^string_view]: [std::basic_string_view](https://en.cppreference.com/w/cpp/string/basic_string_view)

[^string_conversion]: [std::string::operator string_view](https://en.cppreference.com/w/cpp/string/basic_string/operator_basic_string_view)

[^dataflow-analysis]: [Data-flow analysis](https://en.wikipedia.org/wiki/Data-flow_analysis)

[^live-analysis]: [Live-variable analysis](https://en.wikipedia.org/wiki/Live-variable_analysis)

[^character-literal]: [Character literal](https://en.cppreference.com/w/cpp/language/character_literal)

[^scc]: [Strongly connected component](https://en.wikipedia.org/wiki/Strongly_connected_component)