// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2/thread.h>

#include <cstdio>

int add(int x, int y) safe
{
  return x + y;
}

struct [[unsafe::send]] lmao {};

void tester(lmao) safe
{
}

void thread_constructor() safe
{
  {
    std2::thread t(add, 1, 2);
    t^.join();
  }

  {
    std2::thread t(add, 1, 2);
  }

  // throw a sleep in here so that detached threads run to completion (hopefully)
  // and then valgrind won't complain about any mem leaks
  unsafe std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main() safe
{
  thread_constructor();
}
