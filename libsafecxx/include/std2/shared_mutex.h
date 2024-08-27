// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/box.h>
#include <std2/utility.h>

#include <shared_mutex>
#include <new>

namespace std2
{

template<class T+>
class
[[unsafe::send(T~is_send), unsafe::sync(T~is_send)]]
shared_mutex
{
  using mutex_type = unsafe_cell<std::shared_mutex>;

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
    friend class shared_mutex;

    shared_mutex const^/a m_;

    lock_guard(shared_mutex const^/a m) noexcept safe
      : m_(m)
    {
    }

    public:
    ~lock_guard() safe {
      unsafe { m_->mtx_->get()&->unlock(); }
    }

    T const^ borrow(self const^) noexcept safe {
      unsafe { return ^*self->m_->data_.get(); }
    }

    T^ borrow(self^) noexcept safe {
      unsafe { return ^*self->m_->data_.get(); }
    }

    T^ operator*(self^) noexcept safe {
      return self.borrow();
    }

    T const^ operator*(self const^) noexcept safe {
      return self.borrow();
    }
  };

  class shared_lock_guard/(a)
  {
    friend class shared_mutex;

    shared_mutex const^/a m_;

    shared_lock_guard(shared_mutex const^/a m) noexcept safe
      : m_(m)
    {
    }

    public:
    ~shared_lock_guard() safe {
      // TODO: it seems hard to get implementations to trigger tools like helgrind, drd
      // here, even with assertions enabled in libstdc++
      // this was mistakenly a call to `->unlock()` which is incorrect
      // we need some method of verifying we get a failure here if we call the wrong thing
      unsafe { m_->mtx_->get()&->unlock_shared(); }
    }

    T const^ borrow(self const^) noexcept safe {
      unsafe { return ^*self->m_->data_.get(); }
    }

    T const^ operator*(self const^) noexcept safe {
      return self.borrow();
    }
  };

  shared_mutex(T data) noexcept safe
    : data_(rel data)
    , unsafe mtx_(init_mtx())
  {
  }

  shared_mutex(shared_mutex const^) = delete;

  lock_guard lock(self const^) safe  {
    unsafe { self->mtx_->get()&->lock(); }
    return lock_guard(self);
  }

  shared_lock_guard lock_shared(self const^) safe {
    unsafe { self->mtx_->get()&->lock_shared(); }
    return shared_lock_guard(self);
  }
};

} // namespace std2
