// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>

#include <cstdio>

#include "lightweight_test.h"

int add(std2::arc<std2::mutex<int>> mtx, int x, int y) safe
{
  auto z = x + y;
  int^ r;
  {
    auto guard = mtx->lock();
    r = mut guard.borrow();
    *r = z;
  }
  drp mtx;
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
  std2::arc<std2::mutex<int>> mtx{std2::mutex(1337)};

  static_assert(decltype(add)~is_send);
  static_assert(std2::mutex<int>~is_send);
  static_assert(std2::mutex<int>~is_sync);
  static_assert(std2::arc<std2::mutex<int>>~is_send);
  static_assert(std2::arc<std2::mutex<int>>~is_sync);

  {
    std2::thread t(add, cpy mtx, 1, 2);

    int r = *mtx->lock();
    if (r != 1337) REQUIRE_EQ(r, 1 + 2);

    t rel.join();
  }

  // test detachment on drop
  {
    std2::thread t(add, cpy mtx, 1, 2);
    unsafe { std::this_thread::sleep_for(std::chrono::milliseconds(250)); }
  }

  {
    std2::thread t(add, cpy mtx, 2, 1);
  }

  {
    std2::thread t(send_callable{}, 24);
    t rel.join();
  }

  // throw a sleep in here so that detached threads run to completion (hopefully)
  // and then valgrind won't complain about any mem leaks
  unsafe { std::this_thread::sleep_for(std::chrono::seconds(1)); }
}

static_assert(std2::mutex<send_callable>~is_send);
static_assert(std2::mutex<send_callable>~is_sync);
static_assert(!std2::mutex<nonsend_callable>~is_send);
static_assert(!std2::mutex<nonsend_callable>~is_sync);

void adder(std2::arc<std2::mutex<int>> m) safe
{
  unsafe { std::this_thread::sleep_for(std::chrono::milliseconds(250)); }
  for (int i = 0; i < 10'000; ++i) {
    auto guard = m->lock();
    int^ x = mut guard.borrow();
    *x += 1;
  }
  drp m;
};

void mutex_test() safe
{
  std2::vector<std2::thread> threads = {};
  std2::arc<std2::mutex<int>> sp{std2::mutex(0)};

  int const num_threads = 8;
  for (int i = 0; i < num_threads; ++i) {
    threads^.push_back(std2::thread(adder, cpy sp));
  }

  for(std2::thread t : rel threads) {
    t rel.join();
  }

  int const val = *sp->lock()^.borrow();
  auto const expected = num_threads * 10'000;
  REQUIRE_EQ(val, expected);
}

void shared_mutex_test() safe
{
  using value_type = std2::box<int>;
  using mutex_type = std2::shared_mutex<value_type>;

  static_assert(mutex_type~is_send);
  static_assert(mutex_type~is_sync);

  static int const num_iters = 10'000;
  static int const num_writer_threads = 4;
  static int const num_reader_threads = 8;
  static int const value = num_writer_threads * num_iters;

  std2::vector<std2::thread> threads = {};
  std2::arc<mutex_type> sp{mutex_type(std2::box(0))};

  using fn_type = void(*)(std2::arc<mutex_type>) safe;

  auto writer = [](std2::arc<mutex_type> sp) safe {
    unsafe { std::this_thread::sleep_for(std::chrono::milliseconds(250)); }

    for (int i = 0; i < num_iters; ++i) {
      unsafe { std::this_thread::yield(); }

      auto guard = sp->lock();
      value_type^ v = mut guard.borrow();
      mut *v.borrow() += 1;
    }

    drp sp;
  };

  auto reader = [](std2::arc<mutex_type> sp) safe {
    unsafe { std::this_thread::sleep_for(std::chrono::milliseconds(250)); }

    int v = 0;
    do {
      unsafe { std::this_thread::yield(); }

      auto guard = sp->lock_shared();
      value_type const^ v2 = *guard;
      v = **v2;
    } while(v < value);

    drp sp;
  };

  unsafe { fn_type fp1 = +writer; }
  unsafe { fn_type fp2 = +reader; }

  for (int i = 0; i < num_writer_threads; ++i) {
    mut threads.push_back(std2::thread(fp1, cpy sp));
  }

  for (int i = 0; i < num_reader_threads; ++i) {
    mut threads.push_back(std2::thread(fp2, cpy sp));
  }

  for (std2::thread t : rel threads) {
    t rel.join();
  }

  REQUIRE_EQ(**sp->lock_shared(), value);
}

TEST_MAIN(
  thread_constructor,
  mutex_test,
  shared_mutex_test
)
