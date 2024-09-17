#feature on safety

#include <std2.h>
#include "lightweight_test.h"

struct anon_callable/(a, b, c)
{
  std2::string_view/c sv_;
  int^/a x_;
  int const^/b y_;

  anon_callable(
    std2::string_view/c sv,
    int^/a x,
    int const^/b y) safe
    : sv_(sv)
    , x_(x)
    , y_(y)
  {
  }

  [[unsafe::drop_only(a, b)]]
  ~anon_callable() = default;

  void operator()(self const^) safe
  {
    self->sv_;
    self->x_;
    self->y_;
  }
};

void functor_test() safe
{
  std2::string_view sv = "hello, world!"sv2;

  int a = 1234;
  int const b = 4321;

  anon_callable f(sv, ^a, b);
  f();
}

void drop_only() safe
{
  {
    std2::arc<std2::string_view> p;
    {
      std2::string s("hello, world!");

      // TODO: re-enable this test once we get pointer variance working
      p = std2::arc(s.str());
      REQUIRE_EQ(*p, "hello, world!"sv2);
    }
  }
}

TEST_MAIN(functor_test, drop_only)
