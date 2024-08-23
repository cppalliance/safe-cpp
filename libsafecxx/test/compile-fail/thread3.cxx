// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2/thread.h>

struct [[unsafe::send(false)]] not_send {
  int x_;

  not_send() safe
    : x_{0}
  {}

  int operator()(self const^, int x) safe {
    return x + 1;
  }
};

int main() safe
{
  std2::thread t(not_send{}, 42);
}