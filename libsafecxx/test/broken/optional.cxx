// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// ❯ circle libsafecxx/test/broken/optional.cxx -I libsafecxx/include/
// Both operands to ICmp instruction are not of the same type!
//   %cond = icmp eq %union.anon %3, i8 1
// in function _ZN4std28optionalINS_3boxIiEEED1Ev
// LLVM ERROR: Broken function found, compilation aborted!
// fish: Job 1, 'circle libsafecxx/test/broken/o…' terminated by signal SIGABRT (Abort)

#feature on safety

#include <std2.h>

void optional_accessors() safe
{
  {
    std2::optional<std2::box<int>> mp = .some(std2::box{1234});
  }
}

int main() safe
{
  optional_accessors();
}
