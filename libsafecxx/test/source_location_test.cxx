// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2/source_location.h>
#include <cstdio>

template<class T, class U>
void assert_eq(const T^ t, const U^ u) safe
{
  if (*t != *u) throw "unequal values";
}

void assert_true(bool b) safe
{
  if (!b) throw "failed boolean assertion";
}

void source_location() safe
{
  char buf[] = {'l','m','a','o'};
  auto loc = std2::source_location::current(buf);
  unsafe { printf("%s\n", loc.file_name()); }
}

int main() {
  source_location();
}
