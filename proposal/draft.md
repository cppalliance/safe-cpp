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

Security professionals urge projects to migrate away from C++ and adopt memory safe languages. But the scale of the problem is daunting. C++ powers software that has generated trillions of dollars of value. There are a lot of C++ programmers and a lot of C++ code. Given how wide-spread C and C++ code is, what can industry really do to improve software quality and reduce vulnerabilities? What are the options for introducing new memory safe code into existing projects and hardening software that already exists?

There's only one popular systems-level/non-garbage collected language that provides rigorous memory safety. That's the Rust language.[^rust-language] But while they play in the same space, C++ and Rust are idiomatically very different with limited interop capability, making incremental migration from C++ to Rust a slow, painstaking process.

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

Rigorous safety is a carrot-and-stick approach. The stick comes first. The stick is what regulators care about. Developers are prohibited from writing operations that may result in lifetime safety, type safety or thread safety undefined behaviors. Sometimes these operations are prohibited by the compiler frontend, as is the case with pointer arithmetic. Sometimes the operations are prohibited by static analysis in the compiler's middle-end; that stops use of initialized variables and use-after-free bugs, and it's the technology enabling the _ownership and borrowing_ safety model. The remainder of issues, like out-of-bounds array subscripts, are averted with runtime panic and aborts.

The carrot is a suite of new capabilities which improve on the unsafe ones denied to users. The affine type system (aka linear types, aka relocation, aka destructive move) makes it easier to relocate objects without breaking type safety. Pattern matching is safe and expressive, and interfaces with the system's new choice types. Borrow checking[^borrow-checking] is the most sophisticated part of the extension, providing a new reference type that flags use-after-free and iterator invalidation defects at compile time.

What are the properties we're trying to deliver with Safe C++?

* A superset of C++ with a _safe subset_. Undefined behavior is prohibited from originating in the safe subset.
* The safe and unsafe parts of the language are clearly delineated, and users must explicitly leave the safe context to use unsafe operations.
* The safe subset must remain _useful_. If we get rid of a crucial unsafe technology, like unions or pointers, we should supply a safe alternative, like choice types or borrows. A perfectly safe language is not useful if it's so inexpressive you can't get your work done.
* The new system can't break existing code. If you point a Safe C++ compiler at existing C++ code, that code must compile normally. Users opt into the new safety mechanisms. Safe C++ is an extension of C++. It's not a new language.

[^borrow-checking]: [The Rust RFC Book - Non-lexical lifetimes](https://rust-lang.github.io/rfcs/2094-nll.html)

### A safe program

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

    println(x);
  }
}
```
```txt
SHOW OUTPUT
```

Consider this demonstration of Safe C++ that catches iterator invalidation, a kind of use-after-free bug. Let's break it down line by line:

Line 1: `#feature on safety` - Turn on the new safety-related keywords within this file. Other files in your translation unit are unaffected. This is how Safe C++ avoids breaking existing code--everything is opt-in, including the new keywords and syntax. The safety feature changes the object model for function definitions, enabling object relocation, partial and deferred initialization. It lowers function definitions to mid-level intermediate representation (MIR),[^mir] on which borrow checking is performed to flag potential use-after-free bugs on checked references.

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

It feels only right, in the year 2024, to pass Unicode code points to functions that are typed with `int` and deal with characters. But doing so may crash your application, or worse. While the mistake is the caller's for not reading the documentation and following the preconditions, it's the design that's really at fault. Do not rely on the programmer to closely read the docs before using your function. The safe context provided by memory safe languages prevents usage or authoring of functions like `std::isprint` which exhibit undefined behavior when called with invalid arguments.

Rust's approach to safety[^safe-unsafe-meaning] centers on defining responsibility for enforcing preconditions. In a safe context, the user can call safe functions without compromising program soundness. Failure to read the docs may risk correctness, but it won't risk undefined behavior. When the user wants to call an unsafe function from a safe context, they _explicitly take responsibility_ for sound usage of that unsafe function. The user writes the `unsafe` token as a kind of contract: the user has read the terms and conditions of the unsafe function and affirms that it's not being called in a way that violates its preconditions.

Who is to blame when undefined behavior is detected, the caller or the callee? ISO C++ does not have an answer, making it an unreliable language. But Rust's safety model does: whoever typed out the `unsafe` token is at fault. Safe C++ adopts the same principle. Code is divided into unsafe and safe contexts. Unsafe operations may only occur in unsafe contexts. Dropping from a safe context to an unsafe context requires use of the `unsafe` keyword. This leaves an artifact that makes for easy audits: reviewers search for the `unsafe` keyword and focus their attention there first. Developers checking code into the standard library are even required to write _safety comments_[^safety-comments] before every unsafe block, indicating proper usage and explaining why it's sound.

Consider the design of a future `std2::isprint` function. If it's marked `safe`, it must be sound for all argument values. If it's called with an argument that is out of its supported range, it must fail in a deterministic way: it could return an error code, it could throw an exception or it could panic and abort. Inside the `std2::isprint` implementation, there's probably a lookup table with capabilities for each supported character. If the lookup table is accessed with a slice, an out-of-bounds access will implicitly generate a bounds check and panic and abort on failure. If the lookup table is accessed through a pointer, the implementer writes the `unsafe` keyword, drops to the unsafe context, tests the subscript against the range of the lookup table, and fetches the data. The `unsafe` keyword is the programmer's oath that the subsequent unsafe operations are sound.

In ISO C++, soundness holes often occur because caller and callee don't agree on who should enforce preconditions, so neither of them do. In Safe C++, there's a convention backed up by the compiler, eliminating this confusion and improving software quality.

[^isprint]: [`std::isprint`](https://en.cppreference.com/w/cpp/string/byte/isprint)

[^safe-unsafe-meaning]: [How Safe and Unsafe Interact](https://doc.rust-lang.org/nomicon/safe-unsafe-meaning.html)

[^safety-comments]: [Safety comments policy](https://std-dev-guide.rust-lang.org/policy/safety-comments.html)

## Categories of safety

It's instructive to break the memory safety problem down into five categories. Each of these is addressed with a different strategy.

### 1. Lifetime safety

How do we ensure that dangling references are never used? There are two safe technologies: garbage collection and borrow checking. Garbage collection is simple to implement and use, but moves object allocations to the heap, making it incompatible with manual memory manegement. It extends object lifetimes as long as there are live references to them, making it incompatible with C++'s RAII[^raii] object model.

Borrow checking is an advanced form of live analysis. It keeps track of the _live references_ (meaning those that have a future use) at every point in the function, and errors when there's a _conflicting action_ on a place associated with a live reference. For example, writing to, moving or dropping an object with a live shared borrow will raise a borrow checkerror. Pushing to a vector with a live iterator will raise an iterator invalidation error. This is a good system for C++, because it's compatible with manual memory management and RAII.

Borrow checking a function only has to consider the body of that function. It avoids whole-program analysis by instituting the _law of exclusivity_. Checked references (borrows) come in two flavors: mutable and shared, noted respectively as `T^` and `const T^`. There can be one live mutable reference to a place, or any number of shared references to a place, but not both at once. Upholding this principle makes it much easier to reason about your program. Since the law of exclusivity prohibits mutable aliasing, if a function is passed a mutable reference and some shared references, you can be certain that the function won't have side effects that, through the mutable reference, cause the invalidation of those shared references. 

[^dangling-pointer]: [Dangling pointer](https://en.wikipedia.org/wiki/Dangling_pointer)

[^raii]: [Resource acquisition is initialization](https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization)

### 2. Type safety - null pointer variety

> I call it my billion-dollar mistake. It was the invention of the null reference in 1965. At that time, I was designing the first comprehensive type system for references in an object oriented language (ALGOL W). My goal was to ensure that all use of references should be absolutely safe, with checking performed automatically by the compiler. But I couldn't resist the temptation to put in a null reference, simply because it was so easy to implement. This has led to innumerable errors, vulnerabilities, and system crashes, which have probably caused a billion dollars of pain and damage in the last forty years.
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

[^hoare]: [Null References: The Billion Dollar Mistake](https://www.infoq.com/presentations/Null-References-The-Billion-Dollar-Mistake-Tony-Hoare/)

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



## Tour of Safe C++

Put at least 10 examples and describe their opreations. It's best to focus on the code early.


From st louis ISO talk:

1. safe1.cxx - out-of-bounds panic
1. sv1.cxx - dangling string_view
1. sv3.cxx - 
1. iter2.cxx - iterator invalidation
1. safe3.cxx - deref






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

## The `unsafe` type qualifier

The Rust ecosystem was built from the bottom-up prioritizing safe code. Consequently, there's so little unsafe code that the _unsafe-block_ is generally sufficient for interfacing with it. By contrast, there are many billions of lines of unsafe C++ in the wild. The _unsafe-block_ isn't powerful enough to interface our safe and unsafe assets, as we'd be writing _unsafe-blocks_ everywhere, making a noisy mess. Worse, we'd be unable to use unsafe types from safe function templates, since the template definition wouldn't know it was dealing with unsafe template parameters. Because of the ecosystem difference, Rust does not provide guidance for this problem, and we're left to our own devices.

**Safe C++'s answer to safe/unsafe interoperability is to make safeness part of the type system.**

C++ has `const` and `volatile` type qualifiers. C++ compilers also support the `_Atomic` type qualifier,[^atomic-types] through C11. Safe C++ adds the `unsafe` type qualifier. Declare an object or data member with the unsafe qualifier and use it freely _even in safe contexts_. The `unsafe` token means the same thing here as it does with _unsafe-blocks_: the programmer is declaring responsibility for upholding the conditions of the object. Blame lies with the `unsafe` wielder.

Naming an unsafe object yields an lvalue expression of the unsafe type. What are the effects of the unsafe qualifier on an expression?

* Calling unsafe member functions on unsafe-qualified objects is permitted.
* Calling unsafe functions where a function argument is unsafe-qualified is permitted.
* Unsafe constructors may initialize unsafe types.

Calling unsafe member functions on expressions with unsafe types is permitted in the unsafe context. Calling initializers of unsafe types is also permitted. In fact, these operations on unsafe types are "safe" for the purpose of _safe-operator_.

Expressions carry noexcept and safe information which is outside of the type's expression; this information is moved transitively between subexpressions and feeds the _noexcept-_- and _safe-operator_. Why make unsafe a type qualifier, which represents a significant change to the type system, rather than some other kind of property of an object or member declaration, propagate it like the noexcept and safe flags? 

The answer is that template specialization works on types and it doesn't work on these other kinds of properties. A template argument with an unsafe qualifier instantiates a template with an unsafe qualifier on the corresponding template parameter. The unsafe qualifier drills through templates in a way that other language entities don't.

```cpp
int main() safe {
  // Requires unsafe type specifier because std::string's dtor is unsafe.
  std2::vector<unsafe std::string> vec;

  // Construct an std::string from a const char* (unsafe)
  // Pass by relocation (unsafe)
  mut vec.push_back("Foo");
  
  // Pass const char*
  // Construct inside emplace_back (unsafe)
  mut vec.push_back("Bar");

  // Append Bar to the end of Foo (unsafe)
  mut vec[0] += vec[1]; 

  std2::println(vec[0]);
}
```

We want to use the new memory-safe vector with the legacy string type. The new vector is borrow checked, eliminating use-after-free and iterator invalidation defects. It presents a safe interface. But the old string is pre-safety. All its member functions are unsafe. If we want to specialize the new vector on the old string, we need to mark it `unsafe`. 

The unsafe type qualifier propagates through the instantiated vector. The expressions returned through the `operator[]` accessor are unsafe qualified, so we can call unsafe member functions on the string, even in main's safe context.

Let's simplify the example above and study it in detail.

```cpp
#feature on safety 

template<typename T+>
struct Vec {
  Vec() safe;
  void push_back(self^, T obj) safe;
};

struct String {
  String(const char*); // Unsafe ctor.
};

int main() safe {
  // Vec has a safe constructor.
  Vec<unsafe String> vec { };

  // void Vec<unsafe String>::push_back(self^, unsafe String) safe;
  // Copy initialization of the `unsafe String` function parameter is 
  // permitted.
  mut vec.push_back("A string");
}
```

In this example, the unsafe String constructor is called in the safe main function. That's permitted because substitution of `unsafe String` into Vec's template parameter creates a push_back specialization with an `unsafe String` function parameter. Safe C++ allows unsafe constructors to initialize unsafe types in an unsafe context.

Permitting unsafe operations with unsafe specialization is far preferable to using conditional _unsafe-specifiers_ on the class template's member functions. We want the vector to keep its safe interface so that it can be used by safe callers. This device allows member functions to remain safe without resorting to _unsafe-blocks_ in the implementations. There's a single use of the `unsafe` token, which makes for simple audits during code review.

Placing the unsafe token on the _template-argument-list_, where the class template gets used, is also far safer than enclosing operations on the template parameter type in _unsafe-blocks_ inside the template. In the former case, the user of the container can read its preconditions and swear that the precondidions are met. In the latter case, the template isn't able to make any statements about properly using the template type, because it doesn't know what that type is. The `unsafe` token should go with the caller, not the callee.

```cpp
int main() safe {
  // Vec has a safe constructor.
  Vec<unsafe String> vec { };

  // void Vec<unsafe String>::push_back(self^, unsafe String) safe;
  // This is ill-formed. We can't invoke the unsafe String constructor
  // to initialize an unsafe type.
  mut vec.push_back(String("A string"));
}
```

This code is ill-formed. We've established that it's permitted to copy initialize into the push_back call, since its function parameter is `unsafe String`, but direct initialization of `String` is not allowed. The constructor chosen for direct initialization is unsafe, but the type it's initializing is not. The compiler is right to reject this program because the user is plainly calling an unsafe constructor in a safe context, without a mitigating _unsafe-block_ or unsafe qualifier.

```cpp
#feature on safety 

template<typename T+>
struct Vec {
  Vec() safe;
  void push_back(self^, T obj) safe;

  template<typename... Ts>
  void emplace_back(self^, Ts... obj) safe {
    // Direct initialization with the String(const char*) ctor.
    // This compiles, because T is unsafe-qualified.
    self.push_back(T(rel obj...));
  }
};

struct String {
  String(const char*); // Unsafe ctor.
};

int main() safe {
  // Vec has a safe constructor.
  Vec<unsafe String> vec { };

  // void Vec<unsafe String>::emplace_back(self^, const char*) safe;
  mut vec.emplace_back("A string");
}
```

This program is well-formed. As with the previous example, there's a direct initialization of a String object using its unsafe constructor. But this time it's allowed, because the type being initialized is `T`, which is substituted with `unsafe String`: unsafe constructors are permitted to initialize unsafe types. 

[^atomic-types]: [_Atomic types](https://en.cppreference.com/w/c/language/atomic)

### Exempted calls

In order to be more accommodating of mixing unsafe with safe code, the unsafe qualifier has very liberal transitive properties. A function invoked with an unsafe-qualified object or argument, or a constructor that initializes an unsafe type, are _exempted calls_. When performing overload resolution for exempted calls, function parameters of candidates become unsafe qualified. This permits copy initialization of 

TODO

### _using-unsafe-declaration_

## Lifetime safety

There's one widely deployed solution to lifetime safety: garbage collection. In GC, the scope of an object is extended as long as there are live references to it. When there are no more live references, the system is free to destroy the object. Most memory safe languages use tracing garbage collection.[^tracing-gc] Some, like Python and Swift, use automatic reference counting,[^arc] a flavor of garbage collection with different tradeoffs.

Garbage collection requires storing objects on the _heap_. But C++ is about _manual memory management_. We need to track references to objects on the _stack_ as well as on the heap. As the stack unwinds objects are destroyed. We can't extend their duration beyond their lexical scopes. Borrow checking[^borrow-checking] is a kind of compile-time analysis that prevents using a reference after an object has gone out of scope. That is, it solves use-after-free and iterator invalidation bugs.

[^tracing-gc]: [Tracing garbage collection](https://en.wikipedia.org/wiki/Tracing_garbage_collection)

[^arc]: [Automatic reference counting](https://docs.swift.org/swift-book/documentation/the-swift-programming-language/automaticreferencecounting/)

### Use-after-free

`std::string_view`[^string_view] was added to C++ as a safer alternatives to passing character pointers around. Unfortunately, it's so safe unsafe that its reported to _encourage_ use-after-free bugs.[^string-view-use-after-free]

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

Safe C++ allows us to author lifetime-aware `string_view` types that provide memory safety. The compiler prohibits uses of dangling views.

```cpp
#feature on safety
#include <std2/string.h>

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

[^string_view]: [std::basic_string_view](https://en.cppreference.com/w/cpp/string/basic_string_view)

[^string-view-use-after-free]: [std::string_view encourages use-after-free; the Core Guidelines Checker doesn't complain](https://github.com/isocpp/CppCoreGuidelines/issues/1038)

[^string_conversion]: [std::string::operator string_view](https://en.cppreference.com/w/cpp/string/basic_string/operator_basic_string_view)

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

[^dataflow-analysis]: [Data-flow analysis](https://en.wikipedia.org/wiki/Data-flow_analysis)

[^live-analysis]: [Live-variable analysis](https://en.wikipedia.org/wiki/Live-variable_analysis)

### Systems of constraints

NLL borrow checking,[^borrow-checking] is Rust's intentive method for testing invalidating actions against live borrows in the presence of control flow, re-assignments and function calls. The algorithm involves generating a system of lifetime constraints which map borrow variables back to _loans_, growing points until all the constraints are satisfied, and then testing invalidating actions against all loans in scope. 

A loan is the action that forms a borrow to a place. In the example above, there are two loans: `^x` and `^y`. Solving the constraint equation extends the liveness of loans `^x` for and `^y` up until the point of the last dereferences to them. When `y` goes out of scope, it doesn't invalidate the loan `^x`, because that's not live. But it does invalidate the loan `^y`, which is lifetime extended by the final `f(*ref)` expression.

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

The law of exclusivity is enforced at this point. A new mutable loan is an invalidating action on loans that are live at an overlapping place. A new shared loan is an invalidating action on mutable loans that are live at an overlapping place. Additionally, storing to variables is always an invalidating action when there is any loan, shared or mutable, on an overlapping place.

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

Rust uses single-quotes to introduce lifetime parameters and lifetime arguments. That's not a workable choice for us, because C supports multi-character literals.[^character-literal] This cursed feature, in which literals like `'abcd'` evaluate to constants of type `int`, makes lexing Rust-style lifetime arguments very messy.

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

[^character-literal]: [Character literal](https://en.cppreference.com/w/cpp/language/character_literal)

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

The compiler maps all non-dependent types to canonical types. When comparing types for equality, it compares the pointers to their canonical types. This is necessary to support typedefs and alias templates that appear in functions--we need to strip away those inessential details and get to the canonical types within. The lifetime parameterizations the user writes also map to canonical pameterizations.

Think about lifetime parameterizations as a directed graph. Lifetime parameters are the nodes and outlives constraints define the edges. The compiler finds the strongly connected components[^scc] of this graph. That is, it identifies all cycles and reduces them into SCC nodes. In `F4`, the `/a` and `/b` lifetime parameters constrain one another, and there are collapsed into a strongly connected component. The canonical function type is encoded using SCCs as lifetime parameters. Both `F4` and `F5` map to the same canonical type, and therefore compare the same.

During the type relation pass that generates lifetime constraints for function calls in the MIR, arguments and result object regions are constrained to regions of the canonical type's SCCs, rather than the lifetime parameters of the declared type. This reduces the number of regions the borrow checker solves for. But the big reason for this process is to permit writing compatible functions even in the face lifetime normalization.  

[^scc]: [Strongly connected component](https://en.wikipedia.org/wiki/Strongly_connected_component)

### Lifetimes and templates

### Lifetime normalization

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

[^two-phase]: [Two-phase borrows](https://rustc-dev-guide.rust-lang.org/borrow_check/two_phase_borrows.html#two-phase-borrows)

### The mutable context



## Relocation object model

A core enabling feature of Safe C++ is the new object model. It supports relocation/destructive move of local objects, which is necessary for satisfying type safety. Additionally, _all mutations are explicit_. This is nice in its own right, but it's really important for distinguishing between mutable and shared borrows.

```cpp
#feature on safety
#include <std2/box.h>
#include <std2/string.h>

using namespace std2;

int main() safe {
  // No default construct. p is uninitialized.
  box<string> p;

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

The `std2::box` has no default state. It's safe against [null pointer type safety bugs](intro.md#2-type-safety-null-variety). A `box` that's declared without a constructor is not default initialized. It's uninitialized. It's illegal to use until it's been assigned to.

```cpp
#feature on safety
#include <std2/box.h>
#include <std2/string.h>

using namespace std2;

void f(box<string> p) safe { }

int main() safe {
  // No default construct. p is uninitialized.
  box<string> p;
  p = box<string>::make("Hello");
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

Once we assign to `p`, we can use it. But we can't `std::move` into another function, because move semantics put the operand into its default state, and we're stipulating that the box has no default state. Fortunately, the new object model provides for _relocation_, which moves the contents of the object into a value, and sets the source to uninitialized. The destructor on the old object never gets run, because that old declaration no longer owns the object. Ownership has changed with the relocation.

It's a feature, not a defect, that the compiler errors when you use an uninitialized or potentially uninitialized object. The alternative is a type safety error, such as a null pointer dereference undefined behavior.

In Rust, objects are _relocated by default_, unless they implement the `Copy` trait,[^copy-trait] in which case they're copied. If you want to copy a non-Copy object, implement the `Clone` trait[^clone-trait] and call the `clone` member function.

I think implicit relocation is too surprising for C++ users. We're more likely to have raw pointers and legacy references tracking objects, and you don't want to pull the storage out from under them, at least not without some clear token in the source code. That's why Safe C++ includes _rel-expression_ and _cpy-expression_.

* `rel x` - relocate `x` into a new value. `x` is set as uninitialized.
* `cpy x` - copy construct `x` into a new value. `x` remains initialized.

If an object is _trivially copyable_, as all scalars are, then you don't need either of these tokens. The compiler will copy your value. Both _rel-_ and _cpy-expressions_ produce prvalues of the operand's type. 

Why do I make both copy and relocation explicit? I want to make it easy for users to choose the more efficient option. If a type is not trivially copyable, you can opt into an expensive copy with _cpy-expression_. This avoids performance bugs, where an object undergoes an expensive copy just because the user didn't know it was there. Or, if you don't want to copy, use _rel-expression_, which is efficient but destroys the old object, without destructing it.

* `drp x` - call the destructor on an object and set as uninitialized.

Local objects start off uninitialized. They're initialized when first assigned to. Then they're uninitialized again when relocated from. If you want to _destruct_ an object prior to it going out of scope, use _drp-expression_. Unlike Rust's `drop` API,[^drop] this works even on objects that are pinned or are only potentially initialized (was uninitialized on some control flow paths) or partially initialized (has some uninitialized subobjects).

Consider a function like `std::unique_ptr::reset`.[^unique_ptr-reset] It destructs the existing object, if one is engaged, and sets the unique_ptr to its null state. But in our safe version, box doesn't have a default state. It doesn't supply the `reset` member function. Instead, users just drop it, running its destructor and leaving it uninitialized.

You've noticed the nonsense spellings for these keywords. Why not call them `move`, `copy` and `drop`? I wanted to avoid shadowing those common identifiers and improve results when searching code or the web.

[^copy-trait]: [`Copy` in `std::marker`](https://doc.rust-lang.org/std/marker/trait.Copy.html)

[^clone-trait]: [`Clone` in `std::clone`](https://doc.rust-lang.org/std/clone/trait.Clone.html)

[^pin]: [Module `std::pin`](https://doc.rust-lang.org/std/pin/index.html)

[^drop]: [`drop` in `std::ops`](https://doc.rust-lang.org/std/ops/trait.Drop.html)

[^unique_ptr-reset]: [`std::unique_ptr::reset`](https://en.cppreference.com/w/cpp/memory/unique_ptr/reset)


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

We address the defects in C++'s algebraic types by including new first-class tuple, array and [choice](#choice-types) types. Safe C++ is still fully compatible with legacy types, but because of their non-local element access, relocation from their subobjects is not feasible. Relocation is important to type safety, because many types prohibit default states, making C++-style move semantics impossible. Either relocate your object, or put it in an `optional` from which it can be unwrapped.

```cpp
#feature on safety

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
Types are noted with comma-separated lists of types inside parentheses. Expressions are noted with comma-separated lists of expressions inside parentheses. You can nest them. You can access elements of tuple expressions by chaining indices together with dots. Tuple fields are accessed with the customary special tuple syntax: just write the element index after a dot, eg `tup.0`. 

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

### `operator rel`

Safe C++ introduces a new special member function, the _relocation constructor_, written `operator rel(T)`, for all class types. Using the _rel-expression_ invokes the relocation constructor. The relocation constructor exists to bridge C++11's move semantics model with Safe C++'s relocation model. Relocation constructors can be:

* User defined - manually relocate the operand into the new object. This can be used for fixing internal addresses, like those used to implement sentinels in standard linked lists and maps.
* `= trivial` - Trivially copyable types are already trivially relocatable. But other types may be trivially relocatable as well, like `box`, `unique_ptr`, `rc`, `arc` and `shared_ptr`.
* `= default` - A defaulted or implicitly declared relocation constructor is implemented by the compiler with one of three strategies: trivial types are trivially relocated; aggregate types use member-wise relocation; and other types are move-constructed into the new data, and the old operand is destroyed.
* `= delete` - A deleted relocation constructor _pins_ a type. Objects of that type can't be relocated. A `rel-expression` is a SFINAE failure. Rust uses its `std::Pin`[^pin] pin type as a container for structs with with address-sensitive states. That's an option with Safe C++'s deleted relocation constructors. Or, users can write user-defined relocation constructors to update address-sensitive states.

Relocation constructors are always noexcept. It's used to implement the drop-and-replace semantics of assignment expressions. If a relocation constructor was throwing, it might leave objects involved in drop-and-replace in illegal uninitialized states. An uncaught exception in a user-defined or defaulted relocation constructor will panic and terminate.

[^pin]: [Module `std::pin`](https://doc.rust-lang.org/std/pin/index.html)

## Choice types

### Pattern matching

## Thread safety

## Interior mutability

Recall the law of exclusivity, the program-wide invariant that guarantees a resource isn't mutated while another user access it. How does this square with the use of shared pointers, which enables shared ownership of a mutable resource? How does it support threaded programs, where access to shared mutable state is gated by a mutex?

Shared mutable access exists in this safety model, but the way it's enabled involves some trickery. Rust has a blessed type, `UnsafeCell`,[^unsafe-cell] which encapsulates an object and provides mutable access to it, _through a shared reference_. 

```rust
pub const fn get(&self) -> *mut T 
```

This is an official way of stripping away const. While the function is safe, it returns a raw pointer, which is unsafe to dereference. Rust includes a number of library types which wrap `UnsafeCell` and implement their own deconfliction strategies to prevent violations of exclusivity. 

* `std::Cell<T>`[^cell] provides get and set methods to read out the current value and store new values into the protected resource. Since `Cell` can't be used across threads, there's no risk of violating exclusivity.
* `std::RefCell<T>`[^ref-cell] is a single-threaded multiple-read, single-write lock. If the caller requests a mutable reference to the interior object, the implementation checks its counter, and if the object is not locked, it establishes a mutable lock and returns a mutable borrow. If the caller requests a shared reference to the interior object, the implementation checks that there is no live mutable borrow, and if there isn't, increments the counter. When users are done with the borrow, they have to release the lock, which decrements the reference count. If the user's request can't be serviced, the `RefCell` can either gracefully with an error code, or it can panic and abort.
* `std::Mutex<T>`[^mutex] provides mutable borrows to the interior data across threads. A mutex synchronization object deconflicts access, so there's only one live borrow at a time.
* `std::RwLock<T>`[^rwlock] is the threaded multiple-read, single-write lock. The interface is similar to RefCell's, but it uses a mutex for deconfliction, so clients can sit on the lock until their request is serviced.

Safe C++ provides `std2::unsafe_cell` in its standard library. It provides the same interior mutability strategy as Rust:

```cpp
template<class T+>
class [[unsafe::sync(false)]] unsafe_cell
{
  T t_;

public:
  unsafe_cell() = default;
  unsafe_cell(T t) noexcept safe
    : t_(rel t)
  {
  }

  T* get(self const^) noexcept safe {
    return const_cast<T*>(addr self->t_);
  }
};
```

Forming a pointer to the mutable inner state through a shared borrow is _safe_, but dereferencing that pointer is unsafe. Safe C++ implements `std2::cell`, `std2::ref_cell`, `std2::mutex` and `std2::shared_mutex`, which provide safe member functions to access interior state through their deconfliction strategies.





EXAMPLE OF CELL ?



Safe C++ and Rust and equate exclusive access with mutable types and shared access with const types. This is an economical choice, because one type qualifier, const, also determines exclusivity. But this awkward cast-away-const model of interior mutability is the logical consequence. 

[^unsafe-cell]: [UnsafeCell](https://doc.rust-lang.org/std/cell/struct.UnsafeCell.html)
[^cell]: [Cell](https://doc.rust-lang.org/std/cell/struct.Cell.html)
[^ref-cell]: [RefCell](https://doc.rust-lang.org/std/cell/struct.RefCell.html)
[^mutex]: [Mutex](https://doc.rust-lang.org/std/sync/struct.Mutex.html)
[^rwlock]: [RwLock](https://doc.rust-lang.org/std/sync/struct.RwLock.html)
[^ante]: [Ante Shared Interior Mutability](https://antelang.org/blog/safe_shared_mutability/#shared-interior-mutability)

## Implementation guidance

## Unresolved or unimplemented design issues

### _expression-outlives-constraint_

C++ variadics don't convey lifetime constraints from the function's return type to its parameters. Calls like `make_unique` and `emplace_back` take parameters `Ts... args` and return an unrelated type `T`. This may trigger the borrow checker, because the implementation of the function will produce free regions with unrelated endpoints. It's not a soundness issue, but it is a serious usability issue.

We need an _expression-outlives-constraint_, a programmatic version of _outlives-constrant_ `/(where a : b)`. It consists of an _expression_ in an unevaluated context, which names the actual function parameters and harvests the lifetime constraints (and variances?) implied by those expressions. We should name function parameters rather than declvals of their types, because they may be borrows with additional constraints than their template lifetime parameters have.

In order to name the function parameters, we'll need a trailing _expression-lifetime-constraint_ syntax. Something like,

```cpp
template<typename T+, typename... Ts+>
box<T> make_box(Ts... args) safe where(T:T(rel args...));
```

There's a unique tooling aspect to this. To evaluate the implied constraints of the outlives expression, we have lower the expression to MIR, create new region variables for the locals, generate constraints, solve the constraint equation, and propagate region end points up to the function's lifetime parameters.

### Unsafe type qualifier suppression 

### Function parameter ownership

### Relocation out of references

Niko Matsakis writes about a significant potential improvement in the ownership model.[^unwinding-puts-limits-on-the-borrow-checker] You can only relocate out of _owned places_, and owned places are subobjects of local variables. Dereferences of borrows are owned places. But there are situations where it would be sound to relocate out of a reference, as long as you relocate back into it before the function returns.

```rust
fn swap<T>(
    a: &mut T,
    b: &mut T,
) {
    let tmp = *a;
    *a = *b;
    *b = tmp;
}
```

The blog post considers this swap function, which is currently unsupported by Rust.

```cpp
template<typename T>
void swap(T^ a, T^ b) noexcept safe {
  T tmp = rel *a;
  *a = rel *b;
  *b = rel tmp;
}
```

It's equivalent to this function, written in Safe C++'s syntax. This code doesn't compile under Rust or Safe C++ because the operand of the relocation is a dereference, which is not an _owned place_. This defeats the abilities of initialization analysis. 

In Rust, every function call is potentially throwing, including destructors. In some builds, panics are throwing, so array subscripts can exit a function on the cleanup path. Even worse, in debug builds, integer arithmetic may panic to protect against overflow. There are many non-return paths out functions, and unlike C++, it lacks a _noexcept-specifier_ to disable cleanup. Matsakis suggests that relocating out of references is not implemented, because its use would be limited by the many unwind paths out of a function, making it rather uneconomical to support.

It' already possible to write C++ code that is much less burdened by cleanup paths than Rust. If Safe C++ adopted the `throw()` specifier from the Static Exception Specification,[^static-exception-specifications] we could statically verify that functions don't have internal cleanup paths. Reducing cleanup paths extends the interval between relocating out of a reference and restoring an object there, helping justify the cost of more complex initialization analysis. 

I feel this relocation feature is some of the best low-hanging fruit for improving the safety experience in Safe C++. 

[^unwinding-puts-limits-on-the-borrow-checker]: [Unwinding puts limits on the borrow checker
](https://smallcultfollowing.com/babysteps/blog/2024/05/02/unwind-considered-harmful/#unwinding-puts-limits-on-the-borrow-checker)

[^static-exception-specification]: [P3166R0: Static Exception Specifications](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3166r0.html)

