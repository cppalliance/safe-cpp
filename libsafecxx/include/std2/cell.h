// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/optional.h>
#include <std2/utility.h>

namespace std2
{

// Interior mutability is only well-defined through `&UnsafeCell<T>`
// so leaking a `&T` from anywhere in the `Cell` here is an inherent
// soundness hole, which includes calling user-defined copy constructors
// The library can only interact with the underlying T via compiler-generated
// means such as trivial relocation.

template<class T+>
class [[unsafe::sync(false)]] cell
{
  unsafe_cell<T> t_;

  public:

  explicit
  cell(T t) noexcept safe
    : t_(rel t)
  {
  }

  T get(self const^) safe {
    // rely on implicit copy operator erroring out for types with non-trivial
    // destructors or types that have user-defined copy constructors
    unsafe { return *self->t_.get(); }
  }

  void set(self const^, T t) safe {
    unsafe { *self->t_.get() = rel t; }
  }

  T replace(self const^, T t) safe {
    unsafe { auto old = __rel_read(self->t_.get()); }
    unsafe { __rel_write(self->t_.get(), rel t); }
    return old;
  }
};

} // namespace std2
