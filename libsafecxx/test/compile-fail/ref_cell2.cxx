// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>

void compile_fail_impl(std2::ref_cell<int>::ref_mut h) safe
{
  int^ x1 = mut *h;
  int const^ x2 = *h;
  *x1 = 1337;
  if (*x2 != 1337) throw 2;
}

void compile_fail() safe
{
  std2::ref_cell<int> rc{-1};
  match (rc.try_borrow_mut()) {
    .some(x) => compile_fail_impl(rel x);
    .none => throw 1;
  };
}

int main() safe
{
  compile_fail();
}
