// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/cell.h>
#include <std2/optional.h>
#include <std2/utility.h>

#include <cstdint>

namespace std2
{

template<class T+>
class ref_cell
{

  unsafe_cell<T> t_;
  cell<std::int64_t> borrow_count_;

  public:

  class ref
  {};

  class ref_mut
  {};

  explicit
  ref_cell(T t) noexcept safe
    : t_(rel t)
    , borrow_count_{0}
  {
  }

  // optional<ref> try_borrow(self const^) safe
  // {}

  // optional<ref_mut> try_borrow(self^) safe {}
};

} // namespace std2
