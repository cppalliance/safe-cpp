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
