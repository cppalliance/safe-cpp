// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2/thread.h>
#include <std2/mutex.h>

#include <cstdio>

#include "helpers.h"

static std2::mutex<int> const mtx = 1337;

int add(int x, int y) safe
{
  auto z = x + y;
  int^ r;
  {
    auto guard = mtx.lock();
    r = guard^.operator*();
    *r = z;
  }
  return z;
}

struct send_callable
{
  int x_;

  send_callable() safe
    : x_{42}
  {}

  int operator()(self, int x) noexcept safe {
    self.x_ = 24;
    return self.x_ + x;
  }
};

static_assert(send_callable~is_send);

void thread_constructor() safe
{
  {
    std2::thread t(add, 1, 2);

    int r = *mtx.lock();
    if (r != 1337) assert_eq(r, 1 + 2);

    t^.join();
  }

  {
    std2::thread t(add, 1, 2);
  }

  {
    std2::thread t(send_callable{}, 24);
    t^.join();
  }

  // throw a sleep in here so that detached threads run to completion (hopefully)
  // and then valgrind won't complain about any mem leaks
  unsafe std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main() safe
{
  thread_constructor();
}
