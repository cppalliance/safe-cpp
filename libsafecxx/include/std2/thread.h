// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/box.h>
#include <std2/tuple.h>
#include <std2/utility.h>

#include <thread>

#include <cstdio>

namespace std2
{

class thread
{
  std::thread t_;

  template<class F, class ...Args>
  static
  void call((F, (Args...,))* p_tup) safe
  {
    box<(F, (Args...,))> p;
    unsafe p = p_tup;

    // TODO: someday learn this tuple syntax
    auto tup = p rel.into_inner();
    mut tup.0(rel tup.1.[:] ...);
  }

public:

  thread() = delete;
  thread(thread const^) safe = delete;

  template<class F, class ...Args>
  thread(F f, Args... args) safe
  requires(
    F~is_send &&
    (Args~is_send && ...) &&
    safe(mut f(rel args...)))
    : unsafe t_()
  {
    using tuple_type = (F, (Args...,));

    box<tuple_type> p = (rel f, (rel args... ,));
    // TODO: have the thread constructor throw here somehow
    // must catch the case where a clever stdlib dev thinks they can
    // replace p.get() with `p rel.leak()` here, which causes a memory
    // leak upon `std::thread::thread` throwing
    unsafe t_ = std::thread(&call<F, Args...>, p.get());

    forget(rel p);
  }

  ~thread() safe {
    unsafe {
      // TODO: figure out why we need `t_&.` to access `detach()`
      if (t_.joinable()) {
        t_&.detach();
      }
    }
  }

  void join(self^) safe {
    unsafe self->t_&.join();
  }
};

} // namespace std2
