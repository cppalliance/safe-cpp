#feature on safety

#include <std2.h>
#include "helpers.h"

struct borrow_with_drop/(a)
{
  int x_;
  int^/a p_;

  borrow_with_drop(int x, int^/a p) safe
    : x_{x}
    , p_(p)
  {
  }

  [[unsafe::drop_only(a)]]
  ~borrow_with_drop() safe {}
};

void drop_only() safe
{
  {
    std2::string s("hello, world!");
    std2::initializer_list<std2::string_view> list = { s.str() };
    {
      assert_true(list.slice()[0] == "hello, world!"sv2);
      std2::string s2("rawr");
      mut list.slice()[0] = s2.str();
    }
  }

  {
    std2::string s("hello, world!");
    std2::initializer_list<std2::string> list = { rel s };
    {
      assert_true(list.slice()[0] == "hello, world!"sv2);
      std2::string s2("rawr");
      mut list.slice()[0] = rel s2;
    }
  }

  {
    int x = 4321;
    std2::initializer_list<borrow_with_drop> list = { {1234, ^x} };
    {
      int y = 1234;
      mut list.slice()[0] = borrow_with_drop{4321, ^y};
    }
  }
}

int main() safe
{
  drop_only();
}
