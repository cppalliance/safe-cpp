// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>

struct not_send {
  int x_;

  not_send() safe
    : x_{0}
  {}
};

void foo(not_send ns)
{}

static_assert(not_send~is_send);

int main() safe
{
  std2::thread t(foo, not_send{});
}
