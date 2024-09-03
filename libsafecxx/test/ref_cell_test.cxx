// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>

#include "helpers.h"

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
    assert_eq(x.get(), -1);
  }

  {
    std2::cell<copyable> x{42};
    assert_eq(x.get().x_, 42);
  }
}

void cell_mutate() safe
{
  {
    std2::cell<copyable> x{42};
    x.set(copyable{24});
    assert_eq(x.get().x_, 24);

    auto old = x.replace(copyable{1337});
    assert_eq(old.x_, 24);
    assert_eq(x.get().x_, 1337);
  }
}

void verify_ref(std2::ref_cell<int>::ref h) safe
{
  int const^ b1 = *h;
  int const^ b2 = *h;

  assert_eq(b1, b2);

  // TODO: manual drop here seems to cause a double-free with the match block
  // drp h;
}

void ref_cell_constructor() safe
{
  std2::ref_cell<int> rc{-1};
  {
    auto m_x = rc.try_borrow();
    match (m_x) {
      .some(x) => assert_eq(*x, -1);
      .none => assert_true(false);
    };

    auto rc1 = ^const rc;
    auto m_x1 = rc1.try_borrow();
    match (m_x1) {
      .some(x) => assert_eq(*x, -1);
      .none => assert_true(false);
    };

    auto rc2 = ^const rc;
    auto m_x2 = rc2.try_borrow_mut();
    match (m_x2) {
      .some(x) => assert_true(false);
      .none => void();
    };

    auto rc3 = ^const rc;
    auto m_x3 = rc3.try_borrow();
    match (m_x3) {
      .some(x) => verify_ref(rel x);
      .none => assert_true(false);
    };
  }

  {
    auto m_x = rc.try_borrow_mut();
    match (m_x) {
      .some(x) => void(mut *x = 1337);
      .none => assert_true(false);
    };

    auto rc1 = ^const rc;
    auto m_x1 = rc1.try_borrow();
    match (m_x1) {
      .some(x) => assert_true(false);
      .none => void();
    };

    auto rc2 = ^const rc;
    auto m_x2 = rc2.try_borrow_mut();
    match (m_x2) {
      .some(x) => assert_true(false);
      .none => void();
    };
  }

  auto^ p = mut rc.get_mut();
  assert_eq(*p, 1337);
}

int main() safe
{
  cell_constructor();
  cell_mutate();
  ref_cell_constructor();
}
