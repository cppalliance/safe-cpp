// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/optional.h>

namespace std2
{

interface iterator {
  typename item_type;
  optional<item_type> next(self^) safe;
};

interface make_iter {
  typename iter_type;
  typename iter_mut_type;
  typename into_iter_type;

  iter_type      iter(const self^) safe;
  iter_mut_type  iter(self^) safe;
  into_iter_type iter(self) safe;
};

} // namespace std2
