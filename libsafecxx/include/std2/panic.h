// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/source_location.h>
#include <std2/string_view.h>
#include <std2/__panic/codes.h>

#include <cstdio>
#include <cstdlib>

namespace std2
{

// Abort the program
// Panic functions are categorized and marked with an safety::panic(N) attribute.
// This makes it easy for the frontend to toggle on or off panic calls on a
// per-file basis.
[[noreturn, safety::panic(panic_code::generic)]]
inline void panic(
  str msg, source_location loc = source_location::current()) noexcept safe
{
  unsafe { fprintf(stderr, "function \"%s\" panicked at %s:%d\n%.*s\n", loc.function_name(), loc.file_name(), loc.line(), msg.size(), msg.data()); }
  unsafe { abort(); }
}

[[noreturn, safety::panic(panic_code::bounds)]]
inline void panic_bounds(
  str msg, source_location loc = source_location::current()) noexcept safe
{
  unsafe { fprintf(stderr, "out-of-bounds access in \"%s\", at %s:%d\n%.*s\n", loc.function_name(), loc.file_name(), loc.line(), msg.size(), msg.data()); }
  unsafe { abort(); }
}

} // namespace std2
