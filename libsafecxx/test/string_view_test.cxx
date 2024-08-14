// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2/string_view.hxx>
#include <std2/string_constant.hxx>

template<class T, class U>
void assert_eq(const T^ t, const U^ u) safe
{
  if (*t != *u) throw;
}

void assert(bool b) safe
{
  if (!b) throw;
}

void string_view_constructor() safe
{
  // TODO: this segfaults, should eventually be fixed
  // std2::string_constant<char> sc("rawr");

  std2::string_constant<char> sc = "hello, world!";
  std2::string_view sv = sc;
  assert_eq(sv.size(), (*sc.text())~length);
  assert_eq(sv.data(), (*sc.text())~as_pointer);
  assert(sv == sc);
  assert(!(sv != sc));
}

int main()
{
  string_view_constructor();
}
