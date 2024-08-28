// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/atomic.h>

#include <cstdio>

namespace std2
{

template<class T+>
class
[[unsafe::send(T~is_send && T~is_sync), unsafe::sync(T~is_send && T~is_sync)]]
arc
{
  struct arc_inner;
  arc_inner* unsafe p_;

  struct arc_inner
  {
    manually_drop<T> data_;
    atomic<std::size_t> strong_;
    atomic<std::size_t> weak_;

    arc_inner(T data) noexcept safe
      : data_(rel data)
      , strong_(1)
      , weak_(1)
    {
    }
  };

public:
  explicit
  arc(T t) safe
  {
    unsafe { p_ = static_cast<arc_inner*>(::operator new(sizeof(arc_inner))); }
    new(p_) arc_inner(rel t);
  }

  arc(arc const^ rhs) safe
    : p_(rhs->p_)
  {
    ++p_->strong_;
  }

  ~arc() safe
  {
    std::size_t s = --p_->strong_;
    if (s == 0) {
      unsafe { mut p_->data_.destroy(); }

      std::size_t w = --p_->weak_;
      if (w == 0) {
        unsafe { ::operator delete(p_); }
      }
    }
  }

  T const^ operator->(self const^) noexcept safe {
    return ^*self->p_->data_.get();
  }
};

} // namespace std2
