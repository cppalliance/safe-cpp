// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2/cell.h>

struct not_copy
{
  int x_;

  not_copy() safe
    : x_{-1}
  {
  }

  not_copy(not_copy const^ rhs) safe
    : x_{rhs->x_}
  {
  }
};

int main()
{
  std2::cell<not_copy> c{not_copy{}};
  (void) c.get();
}
