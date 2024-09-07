// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>

int main()
{
  std2::manually_drop<std2::box<int>> p{std2::box(1234)};
  p = std2::box<int>{4321};
}
