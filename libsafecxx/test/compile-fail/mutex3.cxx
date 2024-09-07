// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>

int main() safe
{
  std2::mutex<int> mtx = 14;
  std2::mutex<int> mtx2 = rel mtx;
}
