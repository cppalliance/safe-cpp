// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>

#include "helpers.h"

template<class T+>
choice cow/(a)
{
  owned(T),
  borrowed(T const^/a);

  auto to_mut(self^) safe -> T^ {
    return match(mut *self) {
      .owned(x) => ^x;
      .borrowed(b) => std2::panic("");
    };
  }
};

template<class T+>
choice cow2/(a)
{
  owned(T),
  borrowed(T const^/a);

  auto to_mut(self^) safe -> T^ {
    return match(mut *self) {
      .owned(x) => ^x;
      .borrowed(b) => std2::panic("");
    };
  }
};

void use_cow() safe
{
  {
    std2::string base = "rawr";
    cow<std2::string> str = .borrowed(^const base);

    std2::string^ b = mut str.to_mut();
  }

  {
    cow2<std2::string> str = .owned(std2::string("rawr"));
    mut str.to_mut();
  }
}

int main() safe
{
  use_cow();
}
