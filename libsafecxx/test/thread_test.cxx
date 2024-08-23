// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2/thread.h>
#include <std2/mutex.h>
#include <std2/vector.h>
#include <std2/arc.h>

#include <cstdio>

#include "helpers.h"

static std2::mutex<int> const mtx = 1337;

int add(std2::arc<std2::mutex<int>> mtx, int x, int y) safe
{
  auto z = x + y;
  int^ r;
  {
    unsafe auto guard = mtx.lock();
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

struct [[unsafe::send(false)]] nonsend_callable
{
  int x_;

  nonsend_callable() safe
    : x_{42}
  {}

  int operator()(self, int x) noexcept safe {
    self.x_ = 24;
    return self.x_ + x;
  }
};

void thread_constructor() safe
{
  {
    std2::thread t(add, 1, 2);

    unsafe int r = *mtx.lock();
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

static_assert(std2::mutex<send_callable>~is_send);
static_assert(std2::mutex<send_callable>~is_sync);
static_assert(!std2::mutex<nonsend_callable>~is_send);
static_assert(!std2::mutex<nonsend_callable>~is_sync);

void adder(std2::mutex<int> const^/static m) safe
{
  unsafe std::this_thread::sleep_for(std::chrono::milliseconds(250));
  for (int i = 0; i < 1'000'000; ++i) {
    auto guard = m.lock();
    int^ x = ^*guard^.borrow();
    *x += 1;
  }
};

void mutex_test()
{
  std2::vector<std2::thread> threads = {};
  std2::arc<std2::mutex<int>> sp(std2::mutex<int>(1337));

  int const num_threads = 8;
  for (int i = 0; i < num_threads; ++i) {
    // threads^.push_back(std2::thread(adder, ^mtx));
  }
}

int main()
{
  thread_constructor();
  mutex_test();
}
