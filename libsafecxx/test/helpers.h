// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

template<class T, class U>
void assert_eq(const T^ t, const U^ u) safe
{
  if (*t != *u) throw "unequal values";
}

void assert_true(bool b) safe
{
  if (!b) throw "failed boolean assertion";
}

template<class F>
void assert_throws(F f) safe
{
  bool threw = false;
  try {
    f();
  } catch(...) {
    threw = true;
  }
  assert_true(threw);
}

using namespace std2::literals::string_literals;
