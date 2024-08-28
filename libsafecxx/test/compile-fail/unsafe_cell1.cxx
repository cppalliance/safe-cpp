// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2/box.h>
#include <std2/utility.h>

int main()
{
  std2::unsafe_cell<std2::box<int>> p{std2::box(1234)};
  p = std2::box<int>{4321};
}
