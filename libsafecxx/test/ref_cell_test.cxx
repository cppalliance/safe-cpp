// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>

void ref_cell_constructor() safe
{
  {
    std2::ref_cell<int> rc{-1};
  }
}


int main() safe
{
  ref_cell_constructor();
}
