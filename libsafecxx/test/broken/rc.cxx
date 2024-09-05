// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>

struct safe_list
{
  std2::optional<int> next;

  safe_list() safe
    : next(.none)
  {};
};

int main() safe
{
}
