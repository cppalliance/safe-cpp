// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/box.h>
#include <std2/utility.h>

#include <mutex>
#include <new>

namespace std2
{

template<class T+>
class
[[unsafe::send(T~is_send), unsafe::sync(T~is_send)]]
mutex
{
  using mutex_type = unsafe_cell<std::mutex>;

  unsafe_cell<T> data_;
  box<mutex_type> mtx_;

  static
  box<mutex_type>
  init_mtx()
  {
    auto p = static_cast<mutex_type*>(::operator new(sizeof(mutex_type)));
    new(p) mutex_type();
    return box<mutex_type>(p);
  }

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
      unsafe m_->mtx_->get()&->unlock();
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
    , unsafe mtx_(init_mtx())
  {
  }

  mutex(mutex const^) = delete;

  lock_guard lock(self const^) safe  {
    auto *r = self->mtx_->get();
    unsafe self->mtx_->get()&->lock();
    return lock_guard(self);
  }
};

} // namespace std2
