// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

namespace std2
{

template<class T+>
class manually_drop
{
  T t_;

public:
  manually_drop(T t) safe
    : t_(rel t)
  {
  }

  ~manually_drop() = trivial;
};

template<class T+>
void forget(T t) safe
{
  manually_drop<T>(rel t);
}

} // namespace std2