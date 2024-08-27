// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2/thread.h>

void foo/(a)(int^/a x) safe
{}

int main() safe
{
  int x = 1337;
  std2::thread t(foo, ^x);
}
