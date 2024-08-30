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
  cell<int> borrow_count_;

  public:

  class ref/(a)
  {
    friend class ref_cell;

    T* value_;
    cell<int> const^/a borrow_;

    ref(T* value, cell<int> const^/a borrow) noexcept safe
      : value_(value)
      , borrow_(borrow)
    {
      borrow_.set(borrow_.get() + 1);
    }

    public:

    ~ref() safe {
      auto b = borrow_.get();
      borrow_.set(b - 1);
    }

    T const^ operator*(self const^) noexcept safe {
      unsafe { return *self->value_; }
    }
  };

  class ref_mut/(a)
  {
    friend class ref_cell;

    T* value_;
    cell<int> const^/a borrow_;
    // T^/a __phantom_data;

    ref_mut(T* value, cell<int> const^/a borrow) noexcept safe
      : value_(value)
      , borrow_(borrow)
    {
      borrow_.set(borrow_.get() - 1);
    }

    public:

    ~ref_mut() safe {
      auto b = borrow_.get();
      borrow_.set(b + 1);
    }

    T const^ operator*(self const^) noexcept safe {
      unsafe { return *self->value_; }
    }

    T^ operator*(self^) noexcept safe {
      unsafe { return ^*self->value_; }
    }
  };

  explicit
  ref_cell(T t) noexcept safe
    : t_(rel t)
    , borrow_count_{0}
  {
  }

  optional<ref> try_borrow(self const^) noexcept safe
  {
    auto b = self->borrow_count_.get();
    if (b < 0) return .none;
    return .some(ref{self->t_.get(), self->borrow_count_});
  }

  optional<ref_mut> try_borrow_mut(self const^) noexcept safe
  {
    auto b = self->borrow_count_.get();
    if (b > 0) return .none;
    if (b == -1) return .none;
    return .some(ref_mut{self->t_.get(), self->borrow_count_});
  }

  T^ borrow(self^) noexcept safe {
    unsafe { return ^*self->t_.get(); }
  }
};

} // namespace std2
