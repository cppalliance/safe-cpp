// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>
#include "helpers.h"

void box_constructor() safe
{
  {
    std2::box<int> p(1337);

    assert_eq(mut *p, 1337);
    assert_eq(*p, 1337);

    // Bind a mutable borrow.
    int^ x = mut *p;
    *x = 7331;

    assert_eq(*p, 7331);
  }

  {
    std2::box<std2::box<int>> p(std2::box<int>(1337));
    assert_eq(**p, 1337);
  }
}

// proves niche optimization
static_assert(sizeof(std2::unique_ptr<int>) == sizeof(std2::box<int>));

void unique_ptr_constructor() safe
{
  std2::unique_ptr<int> p = .some(std2::box<int>(1337));
  auto x = match(p) -> int {
    .some(x) => *x;
    .none => 7331;
  };

  assert_eq(x, 1337);

}

void drop_only() safe
{
  {
    std2::box<std2::string_view> p;

    {
      std2::string s("hello, world!");
      p = std2::box<std2::string_view>(s.str());
      assert_true(*p == "hello, world!"sv2);
    }
  }
}

int main()
{
  box_constructor();
  unique_ptr_constructor();
  drop_only();
}
