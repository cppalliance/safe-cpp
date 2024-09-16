// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>
#include "lightweight_test.h"

void source_location() safe
{
  auto loc = std2::source_location::current();
  unsafe { printf("%s\n", loc.file_name()); }
}

TEST_MAIN(source_location)
