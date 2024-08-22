// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

namespace std2
{

interface send {};
interface sync {};

template<class T+>
class manually_drop
{
  T t_;

public:
  manually_drop(T t) noexcept safe
    : t_(rel t)
  {
  }

  ~manually_drop() = trivial;
};

template<class T+>
void forget(T t) noexcept safe
{
  manually_drop<T>(rel t);
}

template<class T+>
class [[unsafe::sync(false)]] unsafe_cell
{
  T t_;

public:
  unsafe_cell() = default;
  unsafe_cell(T t) noexcept safe
    : t_(rel t)
  {
  }

  T* get(self const^) noexcept safe {
    return const_cast<T*>(addr self->t_);
  }
};

} // namespace std2
