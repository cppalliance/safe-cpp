// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>

#include "lightweight_test.h"

void simple() safe
{
  int x = 1234;
  int z = match(x) {
    -1 => 1;
    y if y >= 0 => 1337;
    _ => -1;
  };
  REQUIRE_EQ(z, 1337);
}


int func((int, int) tup) safe {
  const int c = 5;

  return match(tup) {
    [4    , y]           => 200 + y;
    [(c)  , y]           => 300 + y;  // now works
    [x @ 2, y]           => 300 + x + y;
    [x    , y] if(x > y) => 400 + x + y;
    [x    , y]           => 500 + x + y;

  };
}

template<class T+>
choice cow/(a)
{
  owned(T),
  borrowed(T const^/a);

  T into_owned(self) safe {
    return match(self) -> T {
      .owned(x) => rel x;
      .borrowed(b) => cpy b;
    };
  }

  bool is_borrowed(self const^) noexcept safe {
    return match(*self) {
      .owned(_) => false;
      .borrowed(_) => true;
    };
  }

  bool is_owned(self const^) noexcept safe {
    return match(*self) {
      .owned(_) => true;
      .borrowed(_) => false;
    };
  };

  auto to_mut(self^) safe -> T^ {
    return match(mut *self) {
      .owned(x) => ^x;
      .borrowed(b) => std2::panic("");
    };
  }
};

template<class T+>
choice cow2/(a)
{
  owned(T),
  borrowed(T const^/a);

  auto to_mut(self^) safe -> T^ {
    return match(mut *self) {
      .owned(x) => ^x;
      .borrowed(b) => std2::panic("");
    };
  }
};

void use_cow() safe
{
  {
    cow<std2::string> str = .owned(std2::string("rawr"));
    std2::string const^ borrow = match (str) -> std2::string const^ {
      .owned(s) => ^const s;
      .borrowed(x) => x;
    };

    std2::string_view s = *borrow;
    REQUIRE_EQ(s, std2::string_view("rawr"));
  }

  {
    cow<std2::string> str = .owned(std2::string("rawr"));
    REQUIRE(str.is_owned());
    REQUIRE(!str.is_borrowed());

    std2::string s = str rel.into_owned();
    REQUIRE_EQ(s, std2::string_view("rawr"));
  }

  {
    std2::string base("rawr");
    cow<std2::string> str = .borrowed(^const base);
    REQUIRE(str.is_borrowed());
    REQUIRE(!str.is_owned());

    // std2::string^ b = mut str.to_mut();

    std2::string s = str rel.into_owned();
    REQUIRE_EQ(s, std2::string_view("rawr"));
  }

  {
    cow2<std2::string> str = .owned(std2::string("rawr"));
    // mut str.to_mut();
  }
}

TEST_MAIN(simple, use_cow)
