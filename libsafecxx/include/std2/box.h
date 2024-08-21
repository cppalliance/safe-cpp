// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/utility.h>

namespace std2
{

template<class T+>
class [[safety::niche_zero]] box
{
  T* p_;
  T __phantom_data;

public:
  box(T* p) noexcept
    : p_(p)
  {
  }

  box(T t) safe
    : unsafe p_(static_cast<T*>(::operator new(sizeof(T))))
  {
    unsafe __rel_write(p_, rel t);
  }

  ~box() safe
  {
    unsafe {
      T t = __rel_read(p_);
      ::operator delete(p_);
    }
  }

  T^ operator*(self^) noexcept safe {
    unsafe return ^*self->p_;
  }

  const T^ operator*(const self^) noexcept safe {
    unsafe return ^*self->p_;
  }

  T* get(self const^) noexcept safe {
    return self->p_;
  }

  T* leak(self) noexcept safe {
    auto p = self.p_;
    forget(rel self);
    return p;
  }

  T into_inner(self) noexcept safe {
    T t;
    unsafe t = __rel_read(self.p_);
    unsafe ::operator delete(self.p_);
    return rel t;
  }
};

} // namespace std2