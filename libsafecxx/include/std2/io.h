// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/string_view.h>
#include <string>

#include <cstdio>

namespace std2
{

inline void println(string_view sv) safe
{
  unsafe { printf("%.*s\n", sv.size(), sv.data()); }
}

inline void println(string_constant<char> sc) safe
{
  println(sc.text());
}

inline void println(const std::string^ s) safe {
  unsafe { printf("%s\n", s->c_str()); }
}

inline void println(signed char x) safe
{
  unsafe { printf("%hhd\n", x); }
}

inline void println(short x) safe
{
  unsafe { printf("%hd\n", x); }
}

inline void println(int x) safe
{
  unsafe { printf("%d\n", x); }
}

inline void println(long int x) safe
{
  unsafe { printf("%ld\n", x); }
}

inline void println(long long int x) safe
{
  unsafe { printf("%lld\n", x); }
}

inline void println(unsigned char x) safe
{
  unsafe { printf("%hhu\n", x); }
}

inline void println(unsigned short x) safe
{
  unsafe { printf("%hu\n", x); }
}

inline void println(unsigned int x) safe
{
  unsafe { printf("%u\n", x); }
}

inline void println(long unsigned int x) safe
{
  unsafe { printf("%lu\n", x); }
}

inline void println(long long unsigned int x) safe
{
  unsafe { printf("%llu\n", x); }
}

} // namespace std2
