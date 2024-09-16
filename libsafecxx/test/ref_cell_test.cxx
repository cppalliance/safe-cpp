// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>

#include "lightweight_test.h"

struct copyable
{
  int x_;

  copyable(int x) safe
    : x_{x}
  {
  }
};

void cell_constructor() safe
{
  {
    std2::cell<int> x{-1};
    REQUIRE_EQ(x.get(), -1);
  }

  {
    std2::cell<copyable> x{42};
    REQUIRE_EQ(x.get().x_, 42);
  }
}

void cell_mutate() safe
{
  {
    std2::cell<copyable> x{42};
    x.set(copyable{24});
    REQUIRE_EQ(x.get().x_, 24);

    auto old = x.replace(copyable{1337});
    REQUIRE_EQ(old.x_, 24);
    REQUIRE_EQ(x.get().x_, 1337);
  }
}

void verify_ref(std2::ref_cell<int>::ref h) safe
{
  int const^ b1 = *h;
  int const^ b2 = *h;

  REQUIRE_EQ(b1, b2);

  // TODO: manual drop here seems to cause a double-free with the match block
  // drp h;
}

void ref_cell_constructor() safe
{
  std2::ref_cell<int> rc{-1};
  {
    auto m_x = rc.try_borrow();
    match (m_x) {
      .some(x) => REQUIRE_EQ(*x, -1);
      .none => REQUIRE(false);
    };

    auto rc1 = ^const rc;
    auto m_x1 = rc1.try_borrow();
    match (m_x1) {
      .some(x) => REQUIRE_EQ(*x, -1);
      .none => REQUIRE(false);
    };

    auto rc2 = ^const rc;
    auto m_x2 = rc2.try_borrow_mut();
    match (m_x2) {
      .some(x) => REQUIRE(false);
      .none => void();
    };

    auto rc3 = ^const rc;
    auto m_x3 = rc3.try_borrow();
    match (m_x3) {
      .some(x) => verify_ref(rel x);
      .none => REQUIRE(false);
    };
  }

  {
    auto m_x = rc.try_borrow_mut();
    match (m_x) {
      .some(x) => void(mut *x = 1337);
      .none => REQUIRE(false);
    };

    auto rc1 = ^const rc;
    auto m_x1 = rc1.try_borrow();
    match (m_x1) {
      .some(x) => REQUIRE(false);
      .none => void();
    };

    auto rc2 = ^const rc;
    auto m_x2 = rc2.try_borrow_mut();
    match (m_x2) {
      .some(x) => REQUIRE(false);
      .none => void();
    };
  }

  auto^ p = mut rc.get_mut();
  REQUIRE_EQ(*p, 1337);
}

void borrowing() safe
{
  {
    std2::ref_cell<int> rc{1234};

    (void) *rc.borrow();
    (void) *rc.borrow_mut();
  }

  {
    std2::ref_cell<int> rc{1234};

    (void) *rc.borrow_mut();
    (void) *rc.borrow_mut();
  }

}

TEST_MAIN(
  cell_constructor,
  cell_mutate,
  ref_cell_constructor,
  borrowing
)
