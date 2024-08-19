#feature on safety

#include <std2/box.h>

#include "helpers.h"

void box_constructor() safe
{
  std2::box<int> p = 1337;
  // TODO: refactor this once Sean adds syntax support
  assert_eq((^p).operator*(), 1337);
  assert_eq(*p, 1337);

  int^ x = (^p).operator*();
  *x = 7331;

  assert_eq(*p, 7331);
}

int main()
{
  box_constructor();
}
