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
    assert_throws([]() safe
    {
      std2::optional<int> mx = .none;
      std2::expected<int, error_code> mx2 = mx.ok_or(error_code{});
      mx2.unwrap();
    });
  }

  {
    std2::optional<int> mx = .some(-1);
    assert_eq(mx.expect("invalid optional used"), -1);
  }

  {
    assert_throws([]() safe
    {
      std2::optional<int> mx = .none;
      mx.expect("invalid optional used");
    });
  }

  {
    std2::optional<int> mx = .some(-1);
    assert_eq(mx.unwrap(), -1);
  }

  {
    assert_throws([]() safe
    {
      std2::optional<int> mx = .none;
      mx.unwrap();
    });
  }
}

int main() safe
{
  optional_accessors();
}
