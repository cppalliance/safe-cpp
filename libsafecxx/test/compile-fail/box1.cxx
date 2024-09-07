// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>

struct incomplete;

int main()
{
  std2::box<std2::box<int>> p{std2::box<int>{1337}};
  mut *p = 7331;
}
