# Language features

## New keywords

`#feature on safety` enables new keywords:

* `safe`
* `unsafe`
* `cpy`
* `rel`
* `drp`
* `addr`
* `mut`
* `choice`
* `match`
* `interface`
* `dyn`
* `impl`
* `self`

## _safe-specifier_

`safe` is a specifier like _noexcept-specifier_.
`safe` is an operator like _noexcept-operator_.

Inside a safe context:
* Cannot dereference pointers or legacy references.
* Cannot perform pointer arithmetic or difference.
* Cannot use _asm-statement_.
* Cannot access non-const objects with static storage duration.
* Cannot name union alternatives.
* Cannot call unsafe functions, since those may include any of the above.
* Other? What kinds of conversions?

##  _lifetime-parameters-list_ and _where-clauses_

TODO

## Borrow types `T^` with lifetime arguments `T^/a`

TODO

## Relocation operator.

`operator rel()` is the relocation constructor. It has three implicitly-defined versions:
1. `= trivial` - trivial relocation.
1. `= delete` - pinned.
1. `= default` - default relocation.

## rel and cpy expersions

_rel-expression_. Convert lvalue to prvalue, leaving old lvalue uninitialized. This only applies to relocatable places.

_cpy-expression_.  Convert lvalue to prvalue using borrow or copy constructor.

## drp expressions

_drp-expression_. `drp x` drops a place. Making this a language feature brings advantages over Rust's `std::drop` version:
* You can drop pinned objects.
* You can drop potentially initialized, partially initialized, or uninitialized objects.

## manually_drop and forget

Forget currently doesn't have its own keyword. You can build a `ManuallyDrop` type by setting its destructor to trivial:

```cpp
  ~manually_drop() = trivial;
```

Relocate the object into `manually_drop`. When the object goes out of scope, the trivial destructor is run, which forgets the object. This design has defects the _drp-expression_ does not have:
* You can't forget pinned objects.
* You can't forget objects that are not definitely initialized.

## `[[safety::drop_only(args)]]` attribute.

Like Rust's `#[may_dangle]`. When dropping an object, lifetime parameters associated with template parameters named by the `drop_only` attribute are not necessarily "used". Nail down semantics here, because it's not as simple as exempting these lifetime parameters from use. What we're really saying is that subobjects with these lifetime parameters are only dropped from the destructor of the containing class. If that inner drop uses the lifetime parameter, then the outer one will too.

## Borrow constructor

Safe version of copy constructor `T(const T^) safe`.

## New aggregate types. 

We can't use `std::tuple`, `std::pair` and `std::array` with relocation because the accessors use references, and you can't relocate through references.

Tuple `(a, b)`. `()`, `(T,)` and `(T1, T2, T3)` are tuple types. `(,)`, `(x,)` and `(x, y, z)` are tuple expressions.

Array `[T; Size]`. An array with value semantics. You can pass and return through functions and it's copied like `std::array`. Supports subslice operations like `array[index; N]`. Supports range operations like `array[begin .. end]`.

## New choice types.

`choice` is a first-class discriminated union.

```cpp
template<typename T+, typename Err+>
choice expected {
  [[safety::unwrap]] ok(T),
  err(Err)
};

template<typename T+>
choice optional {
  default none,
  [[safety::unwrap]] some(T);

  template<typename Err>
  expected<T, Err> ok_or(self, Err err) noexcept safe {
    return match(self) -> expected<T, Err> {
      .some(x) => .ok(rel x);
      .none    => .err(rel err);
    };
  }

  T expect(self, str msg) noexcept safe {
    return match(self) {
      .some(x) => rel x;
      .none    => panic(msg);
    };
  }

  T unwrap(self) noexcept safe {
    return self rel.expect("{} is none".format(optional~string));
  }
};
```

Use the attribute `[[safety::unwrap]]` to mark an alternative for `return?`, `break?` and `continue?` unwrap semantics.

Use the `default` keyword to indicate the alternative for default initialization. Defaulted alternatives cannot have payload types. Or can they? 

## Pattern matching

The type-safe way to work with choice types is pattern matching.

Define bind modes.
Explain shared borrow taken on arguments during if-guard evaluation--this prevents mutation between if-guard evaluation and the body.

## Range expressions

In std2.h, see the six new standard types:
1. `std2::range` - `begin .. end`
1. `std2::range_from` - `begin ..`
1. `std2::range_to` - `.. end`
1. `std2::range_full` - `..`
1. `std2::range_inclusive` - `begin ..= end`
1. `std2::range_to_inclusive` - `..= end`

## Interfaces

A new customization point. Define an `interface`. Then declare an `impl`. Impls are partial specializations that relate types to interfaces.

## Iterator customization point

The _ranged-for-statement_ now considers a second customization point: `interface make_iter`. If the initializer for the ranged-for implements `std2::make_iter`, `make_iter::iter` is invoked to produce an iterator object. This follows the Rust convention of providing a single `next` function which returns the next element and advances the current pointer. The result object of next must be an `std2::optional`.

# Tentative designs

## Pointer-like reference and borrow semantics

Standard conversions only for const references/borrows. We can implicitly bind an lvalue to a const lvalue reference or const borrow type, but we can't implicitly bind to non-const lvalue references or borrow types. **All mutations are explicit** in this model. This is motivated by preferring const borrows as opposed to mutable borrows: const borrows permit aliasing, so are more permissive. If you want exclusivity, perform an explicit mutation `^x` or `&x`.

Legacy references have pointer-like semantics. Naming an lvalue reference produces an lvalue reference expression. Naming an rvalue reference produces an rvalue reference expression. Calling a function that returns a reference produces a reference-valued expression. Eg, calling `std::move` produces a `prvalue T&&` expression, not an `xvalue T` expression. `std::forward` is obsolete in this model, since you can name the references directly. The point is to make legacy references and borrows operate in the same way. 

Support object-postfix reference operator: `vec^.push(x)` or `vec&.push(x)`. This provides a non-const reference/borrow of the object for a mutating call. The const binding case is handled implicitly by standard conversions.

Rust always requires explicit mutable and shared borrowing, except for the receiver object, where both are implicit. The Safe C++ model only ever requires explicit mutable borrowing. The receiver object is treated symmetrically with object-postfix references.

Is it worth it to treat legacy references the same way? If we use ISO C++ reference semantics, what do we do with reference declarations without an initializer? `int& ref` - does this declaration stand until it is bound with an initializer? Do assignments rebind the reference or copy the data referred to? ISO C++'s awkward reference semantics make `std::optional<T&>` a challenge, but that's handled trivially with the pointer-like reference model.

## Explicit objects

The `self` keyword names an explicit object. It's like `this`, but I'm trying to distinguish pointers, which are unsafe to use, from safe objects with value or borrow semantics. 

```cpp
struct Foo/(a) {
  void f1(Foo^/a self);       // long form.
  void f2(self^);             // abbreviated form.

  void f3(const Foo^/a self); // long form.
  void f4(const self^);       // abbreviated form.

  void f5(self);              // consuming function.
};
```

Is this mechanism needed? Can we instead use `^` and `rel` as new _ref-qualifiers_ on member functions?

```cpp
struct Bar/(a) {
  void f1() ^;                // ref-qualifier form of mutable borrow self.
  void f2() const^;           // ref-qualifier form of shared borrow self.
  void f3() rel;              // ref-qualifier form of consuming function.
};
```

The explicit `self` gives a handy place to put lifetime arguments, but do we ever need lifetime arguments? Can it always take the lifetime parameters from the surrounding class? 

If we use the ref-qualifier form, should we permit naming data members and member functions of the implicit object, or should we still require `self` to access those objects? Currently my implementation uses explicit self for all member functions except ctors and dtors.

## Variances

In the current implementation, pointers and references `T*` and `T&` are bivariant in their lifetimes in T. That means that no lifetime calculations are performed after we hit a legacy reference or pointer. In one sense, this isn't unsound, because it's unsafe to dereference these types, so it's up to the programmer to verify the preconditions. But it's clearly not what is wanted. It makes sense to adopt Rust's convention of covariance for `const T&` and `const T*` and invariance for `T& and T*`. Can we have a better story than using `NonNull` or `Unique` to create covariance access of pointers? It's almost like raw pointers should have explicit variance as part of the type.

## Variances, implied constraints and virtual functions

Are virtual function calls safe? How do we account for additional variances and implied constraints that may be put on the lifetime parameters of the base class from the derived class's implementation? If the base pointer doesn't satisfy those implied constraints, we have a soundness violation. I imagine HRTBs address this same issue, but my knowledge of them is poor.

## Phantom data

Currently phantom data is implemented with a special member name `__phantom_data`. It's never initialized, and it won't show up for data layout or reflection. This gets around having to introduce zero-length types. I feel this design is prone to misuse, since you can write a constructor that compiles but doesn't actually constrain the lifetimes on the phantom data.

## Distinguish bound from unbound types

`int^` is an unbound type that has one lifetime binder. `int^/a` is a bound type that binds one lifetime binder.

`int^^` has two unbound lifetimes. `int^/a ^/b` has two bound lifetimes.

`int^/_` is a borrow bound with the _placeholder lifetime_ `_`.

How does the type system distinguish between types with unbound and bound lifetimes? Currently I support a `+` syntax on template parameters:

```cpp
template<typename T+>
struct Vec;

Vec<int^> vec;
```

After argument deduction, template parameters with a `+` modifier implicitly add placeholder lifetime arguments to the corresponding template argument types. The above is equivalent to:

```cpp
template<typename T>
struct Vec;

Vec<int^/_> vec;
```

The placeholder lifetime on the argument is irrelevant. During borrow checking, those lifetimes are erased and new region are assigned to all bound lifetimes. But it does matter during argument deduction, because Vec's T template parameter should have one associated lifetime parameter, the template lifetime parameter T.0. When a template parameter is specialized on a type, what happens to the type's bound or unbound lifetimes? Are they stripped so the template parameter? is unbound? Are they added so the template parameter is bound? Or are they transfered, so it has bound and unbound lifetimes in the same places as the template argument? Is there some kind of test?

What about the is_same problem?

```cpp
template<typename T1, typename T2>
struct is_same {
  enum { value = false };
};
template<typename T>
struct is_same<T, T> {
  enum { value = true };
};

static_assert(is_same<int^, int^>::value);
```

Should this pass the assert or not? That depends on how unbound lifetimes are handled. If T1 and T2 both deduce to `int^` without bound lifetimes, then they are the same. But if new independent lifetimes are created, say `int^/#0` and `int^/#1`, then they don't compare the same, because their lifetimes differ, and choosing the partial specialization would efface that difference.

Note that expressions always have unbound lifetime. Naming a function parameter or a data member with a bound lifetime yields an lvalue expression where the type has stripped lifetimes. That's because lifetime parameters are only enforced at the function boundary: it's a contract between arguments and the function's SCCs, and between the function's SCCs and the result object.

We want bound lifetimes:
* On function parameter types.
* On function return types.
* On data member types.

We want unbound lifetimes:
* On expression types.

Part of this project will be to establish when bound lifetimes are permitted, when they are required, and when they are forbidden.

## A more assertive claim on lifetime binders

I think my current implementation is mostly correct.

There are two kinds of type template parameters:
1. `typename T` - Specializes types with unbound lifetimes.
1. `typename T+` - Specializes types with bound lifetimes.

You'll want the `T+` variant when writing containers like tuple and vector.

Specializing a `T` type parameter on a type with bound lifetimes should be made _ill-formed_. In the unlikely event you have a type with bound lifetimes, strip them with `T/0`, the null lifetime.

Specializing a `T+` type parameter ignores bound lifetimes on the arguments. It performs deduction, and comes out with an unbound lifetime. It then invents new _template lifetime parameters_ for each lifetime binder in the deduced type. Eg, specializing on `const [string_view]^` creates two template lifetime parameters: `#T0.0` and `#T0.1`, resulting in a deduced type `const [string_view/#T0.0]^/#T0.1`. The invented parameters are assigned in depth-first order.

If the user specializes `tuple<int^/a, int^/b>`, the tuple has invented lifetime parameters `#T0.0.0` and `T0.1.0` (major index, minor index, depth-first ordering). The usage desugars to `tuple<int^, int^>/a/b`. 

Argument deduction in functions seems to fall out naturally. I think here it is okay for `T` to invent template lifetime parameters.

```cpp
template<typename T>
void f(T a, T b);
```

Remember that expressions have types with unbound lifetimes. `f(x, y)` specializes T with unbound lifetimes. The compiler invents enough lifetime parameters to bind the argument. Then the caller, at the MIR level, establishes constraints between the region variables for the argument expression `x` and `y` and the SCCs of the invented lifetime parameters of the function.

## Parameter ownership and ABI

In Itanium ABI, objects that are [non-trivial for the purpose of calls](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#non-trivial) are owned by the caller and passed by reference. On return, the caller calls the destructor. This is incompatible with relocation of function parameters, since that changes the ownership of the parameter, and would cause a double-delete if called naively using Itanium ABI. 

Functions with parameters that are non-trivial for the purpose of calls require a new ABI to support parameter relocation. If the ABI is part of the function's type, there is no incompatibility between ABIs. The ABI of the callee doesn't matter--it will allocate storage for the argument and pass it by address, but it doesn't own the argument. When the call returns, the caller does not destroy those objects.

If the relocate ABI is implicit on `#feature on safety` function declarations, how does it work with function poitners?

There are other options than marking the relocate ABI as part of the function's type. We could mark the parameters as being relocatable. A pass-by-value `self` parameter is implicitly relocatable.

# Ideas to address

## Cheap clone

Rust people complain about not knowing if `.clone()` is cheap or expensive. Safe C++ already has one advantage here: if a type is trivially copyable, then `cpy` or `rel` doesn't have to be used to convert an lvalue to a prvalue; the standard conversion will kick in and you get the trivial copy. In this case, you get clone that's guaranteed "cheap," in the sense that it's just a memcpy. That's because rel is always opt in, rather the default.

Should there be some other mechanism to distinguish cheap vs non-cheap non-trivial copies? 

## f-strings (Rust-style printing)

I've done a lot of work on fstrings in Circle. Currently I'm tokenizing the format specifier inside the frontend after the preprocessor has tokenized the string literal. For more accurate errors, I should move tokenization into the preprocessor. `f"hello {:3} {:4}"` would then not appear as one string token, but as an f-string opener, string fragment tokens, braces and colons, etc, and then an f-string closer.

Internally I have these interface declarations:

```cpp
namespace fmt {

  interface fstring;
  interface write;

  interface display;
}
```

The frontend has to connect f-string evaluation to these interfaces. Rust uses type erasure to keep compile times and compile sizes small, at the cost of execution efficiency. 

## Overloading based on `safe`



# Library

