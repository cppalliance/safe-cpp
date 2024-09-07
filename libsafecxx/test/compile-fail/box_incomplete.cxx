// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>

struct incomplete;

std2::box<incomplete> make_incomplete() safe;

void tester() safe
{
  auto p = make_incomplete();
}

int main()
{
}
