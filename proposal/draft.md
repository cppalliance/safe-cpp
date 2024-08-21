# Safe C++

## Types of safety

## Ownership and borrowing





## Unsafe operations

It is ill-formed to perform the following operations in a safe context:

(Link to test cases in compile-fail for these)
* Call an unsafe function.
* Dereference a pointer or legacy reference.
* Offset a pointer.
* Find the difference between two pointers.
* Access fields of a union.
* Name a mutable object with `static` or `thread_local` storage duration.
* Name a `mutable` member of a const-qualified expression.
* `reinterpret_cast`.
* Use inline assembler.

Talk about unsafe attributes? [[unsafe::drop_only]]?

## Panic operations

### Integral math panic

Several integral math operations signal floating-point unit errors. Languages mark these operations as _undefined behavior_, permitting compilers to optimize around them. Rather than permit undefined behavior, Safe C++ checks for these illegal operations and panics, aborting the program.

* Integer divide or remainder by 0.
* Division of INT_MIN by -1.

### Bounds checking panic

It's unsafe to subscript a pointer. But it's safe to subscript an array or slice object. 

### Library panic
