// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>

#include "helpers.h"

class error_code
{
  public:

  error_code() safe = default;
};

void optional_accessors() safe
{
  {
    std2::optional<int> mx = .some(-1);
    std2::expected<int, error_code> mx2 = mx.ok_or(error_code{});

    assert_eq(mx2.unwrap(), -1);
  }

  {
    std2::optional<int> mx = .some(-1);
    assert_eq(mx.expect("invalid optional used"), -1);
  }

  {
    std2::optional<int> mx = .some(-1);
    assert_eq(mx.unwrap(), -1);
  }

  {
    std2::vector<int> xs{1, 2, 3, 4};
    std2::optional<std2::vector<int>> mp = .some(rel xs);

    std2::vector<int> ys{4, 3, 2, 1, 1, 2, 3, 4};
    mp = .some(rel ys);

    assert_eq((mp rel.unwrap()).size(), 8u);
  }

  {
    std2::optional<std2::box<int>> mp = .some(std2::box<int>{1234});
    mp = .some(std2::box<int>{4321});

    assert_eq(*(mp rel.unwrap()), 4321);
  }
}

void take() safe
{
  {
    std2::optional<std2::box<int>> opt = .some(std2::box{1234});
    auto m_p = mut opt.take();

    assert_true(m_p.is_some());
    assert_true(!m_p.is_none());

    assert_true(opt.is_none());
    assert_true(!opt.is_some());
  }

  {
    std2::optional<std2::box<int>> opt = .none;
    auto m_p = mut opt.take();

    assert_true(m_p.is_none());
    assert_true(!m_p.is_some());

    assert_true(opt.is_none());
    assert_true(!opt.is_some());
  }

  {
    // struct C
    // {
    //   static
    //   bool invoke/(a)(int^/a x) safe {
    //     std2::println("x is: ");
    //     std2::println(*x);
    //     bool b = (*x < 4321);
    //     std2::println(b);
    //     return b;
    //   }
    // };

    // std2::optional<int> opt = .some(1234);
    // auto m_p = mut opt.take_if(addr C::invoke);

    // assert_true(m_p.is_some());
    // assert_true(!m_p.is_none());

    // assert_true(opt.is_none());
    // assert_true(!opt.is_some());
  }
}

int main() safe
{
  optional_accessors();
  take();
}
