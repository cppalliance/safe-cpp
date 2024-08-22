// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2/mutex.h>

int main() safe
{
  std2::mutex<int> mtx = 14;
  int^ x;
  {
    auto guard = mtx.lock();
    x = guard^.operator*();
  }
  *x = 7331;
}
