#feature on safety

#include <std2/box.h>

struct incomplete;

std2::box<incomplete> make_incomplete() safe;

void tester() safe
{
  auto p = make_incomplete();
}

int main()
{
}
