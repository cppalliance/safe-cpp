// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>

#include "lightweight_test.h"

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

    REQUIRE_EQ(mx2.unwrap(), -1);
  }

  {
    std2::optional<int> mx = .some(-1);
    REQUIRE_EQ(mx.expect("invalid optional used"), -1);
  }

  {
    std2::optional<int> mx = .some(-1);
    REQUIRE_EQ(mx.unwrap(), -1);
  }

  {
    std2::vector<int> xs{1, 2, 3, 4};
    std2::optional<std2::vector<int>> mp = .some(rel xs);

    std2::vector<int> ys{4, 3, 2, 1, 1, 2, 3, 4};
    mp = .some(rel ys);

    REQUIRE_EQ((mp rel.unwrap()).size(), 8u);
  }

  {
    std2::optional<std2::box<int>> mp = .some(std2::box<int>{1234});
    mp = .some(std2::box<int>{4321});

    REQUIRE_EQ(*(mp rel.unwrap()), 4321);
  }
}

void take() safe
{
  {
    std2::optional<std2::box<int>> opt = .some(std2::box{1234});
    auto m_p = mut opt.take();

    REQUIRE(m_p.is_some());
    REQUIRE(!m_p.is_none());

    REQUIRE(opt.is_none());
    REQUIRE(!opt.is_some());
  }

  {
    std2::optional<std2::box<int>> opt = .none;
    auto m_p = mut opt.take();

    REQUIRE(m_p.is_none());
    REQUIRE(!m_p.is_some());

    REQUIRE(opt.is_none());
    REQUIRE(!opt.is_some());
  }

  struct C
  {
    static
    bool invoke/(a)(int^/a x) safe {
      bool b = (*x < 4321);
      return b;
    }
  };

  {
    std2::optional<int> opt = .some(1234);
    auto m_p = mut opt.take_if(addr C::invoke);

    REQUIRE(m_p.is_some());
    REQUIRE(!m_p.is_none());

    REQUIRE(opt.is_none());
    REQUIRE(!opt.is_some());
  }

  {
    std2::optional<int> opt = .some(43211234);
    auto m_p = mut opt.take_if(addr C::invoke);

    REQUIRE(!m_p.is_some());
    REQUIRE(m_p.is_none());

    REQUIRE(!opt.is_none());
    REQUIRE(opt.is_some());
  }

  {
    std2::optional<int> opt = .none;
    auto m_p = mut opt.take_if(addr C::invoke);

    REQUIRE(!m_p.is_some());
    REQUIRE(m_p.is_none());

    REQUIRE(opt.is_none());
    REQUIRE(!opt.is_some());
  }
}

TEST_MAIN(optional_accessors, take)
