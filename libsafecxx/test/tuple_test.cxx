// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2/tuple.h>

struct empty {};

static_assert(sizeof(std2::tuple<int, empty>) == sizeof(std2::tuple<int>));

int main(){}
