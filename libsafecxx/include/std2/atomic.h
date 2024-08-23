// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/utility.h>
#include <atomic>

namespace std2
{

template<class T>
class [[unsafe::sync(true)]] atomic
{
  unsafe_cell<T> t_;

public:
  atomic(T t = T()) safe
  : t_(rel t)
  {}

  atomic(atomic const^) = delete;
  operator rel(atomic) = delete;

  T fetch_add(self const^, T op, std::memory_order memory_order = std::memory_order_seq_cst) noexcept safe {
    unsafe return __atomic_fetch_add(self->t_.get(), op, memory_order);
  }
  T fetch_sub(self const^, T op, std::memory_order memory_order = std::memory_order_seq_cst) noexcept safe {
    unsafe return __atomic_fetch_sub(self->t_.get(), op, memory_order);
  }

  T add_fetch(self const^, T op, std::memory_order memory_order = std::memory_order_seq_cst) noexcept safe {
    unsafe return __atomic_add_fetch(self->t_.get(), op, memory_order);
  }
  T sub_fetch(self const^, T op, std::memory_order memory_order = std::memory_order_seq_cst) noexcept safe {
    unsafe return __atomic_sub_fetch(self->t_.get(), op, memory_order);
  }

  void store(self const^, std::memory_order memory_order = std::memory_order_seq_cst) noexcept safe {
    unsafe return __atomic_store_n(self->t_.get(), memory_order);
  }

  T load(self const^, std::memory_order memory_order = std::memory_order_seq_cst) noexcept safe {
    unsafe return __atomic_load_n(self->t_.get(), memory_order);
  }

  T operator++(self const^) noexcept safe {
    return self->add_fetch(1);
  }

  T operator++(self const^, int) noexcept safe {
    return self->fetch_add(1);
  }

  T operator--(self const^) noexcept safe {
    return self->sub_fetch(1);
  }

  T operator--(self const^, int) noexcept safe {
    return self->fetch_add(1);
  }
};

} // namespace std2
