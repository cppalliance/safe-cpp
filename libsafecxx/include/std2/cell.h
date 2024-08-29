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

template<class T+>
class cell
{
  unsafe_cell<T> t_;

  public:

  explicit
  cell(T t) noexcept safe
    : t_(rel t)
  {
  }

  T get(self^) safe {
    unsafe { return *self->t_.get(); }
  }

  void set(self^, T t) safe {
    unsafe { ^*self->t_.get() = rel t; }
  }
};

} // namespace std2
