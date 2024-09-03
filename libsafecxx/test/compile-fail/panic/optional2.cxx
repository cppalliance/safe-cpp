// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2.h>

class error_code
{
  public:

  error_code() safe = default;
};

int main()
{
  std2::optional<int> mx = .none;
  std2::expected<int, error_code> mx2 = mx.ok_or(error_code{});
  mx2.unwrap();
}
