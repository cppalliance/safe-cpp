// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>
#include "lightweight_test.h"

void rc_constructor() safe
{
  {
    std2::rc<int> p{-1};
    REQUIRE_EQ(*p, -1);
  }

  {
    using cell_type = std2::ref_cell<int>;

    std2::rc<cell_type> p{cell_type{1234}};
    auto b = p->borrow();
    REQUIRE_EQ(*b, 1234);

    std2::rc<cell_type> p2 = cpy p;
    auto b2 = p2->borrow();
    REQUIRE_EQ(*b2, 1234);

    REQUIRE_EQ(addr *b, addr *b2);
  }
}

void drop_only() safe
{
  {
    std2::rc<std2::string_view> p;
    {
      std2::string s("hello, world!");

      // TODO: re-enable this test once we get pointer variance working
      // p = std2::rc(s.str());
      // REQUIRE(*p == "hello, world!"sv2);
    }
  }
}

int main() safe
{
  rc_constructor();
  drop_only();
}
