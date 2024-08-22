// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/utility.h>

#include <mutex>

namespace std2
{

template<class T+>
class mutex
{
  unsafe_cell<T> data_;
  unsafe_cell<std::mutex> mtx_;

public:
  class lock_guard/(a)
  {
    friend class mutex;

    mutex const^/a m_;

    lock_guard(mutex const^/a m) noexcept safe
      : m_(m)
    {
    }

  public:
    ~lock_guard() safe {
      unsafe m_->mtx_.get()&->unlock();
    }

    T const^ borrow(self const^) noexcept safe {
      unsafe return ^*self->m_->data_.get();
    }

    T^ borrow(self^) noexcept safe {
      unsafe return ^*self->m_->data_.get();
    }

    T^ operator*(self^) noexcept safe {
      return self.borrow();
    }

    T const^ operator*(self const^) noexcept safe {
      return self.borrow();
    }
  };

  mutex(T data) noexcept safe
    : data_(rel data)
    , unsafe mtx_()
  {
  }

  operator rel(mutex) = delete;

  lock_guard lock(self const^) safe  {
    unsafe self->mtx_.get()&->lock();
    return lock_guard(self);
  }
};

} // namespace std2
