// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/panic.h>
#include <std2/__config/config.h>

namespace std2
{

template<class T+, class E+>
choice expected {
  [[safety::unwrap]] ok(T),
  err(E);

  T unwrap(self) SAFECXX_NOEXCEPT safe {
    return match(self) -> T {
      .ok(t) => rel t;
      .err(e) => panic("unwrapping an error-containing expected");
    };
  }
};

} // namespace std2
