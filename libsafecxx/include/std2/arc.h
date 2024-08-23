// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/atomic.h>

namespace std2
{

template<class T+>
class arc
{
  struct arc_inner;
  arc_inner* p_;

  struct arc_inner
  {
    manually_drop<T> data_;
    atomic<std::size_t> strong_;
    atomic<std::size_t> weak_;

    arc_inner(T data) noexcept safe
      : data_(rel data)
      , strong_(1)
      , weak_(1)
    {}
  };

public:
  arc(T t) safe
  {
    unsafe p_ = static_cast<arc_inner*>(::operator new(sizeof(arc_inner)));
    new(p_) arc_inner(rel t);
  }

  arc(arc const^ rhs) safe
    : p_(rhs.p_)
  {
    unsafe ++p_->strong_;
  }

  ~arc() safe
  {
    std::size_t s;
    unsafe s = --p_->strong_;
    if (s == 0) {
      unsafe p_->data_^.destroy();

      std::size_t w;
      unsafe w = --p_->weak_;
      if (w == 0) {
        unsafe ::operator delete(p_);
      }
    }
  }
};

} // namespace std2
