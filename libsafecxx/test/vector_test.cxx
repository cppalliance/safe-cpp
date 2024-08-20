// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2/vector.h>
#include <std2/box.h>

#include "helpers.h"

void vector_constructor() safe
{
  {
    std2::vector<int> vec{};
    assert_eq(vec.size(), 0u);

    vec^.push_back(1);
    vec^.push_back(2);
    vec^.push_back(3);

    assert_eq(vec.size(), 3u);

    {
      auto s = vec^.slice();
      assert_eq(s[0], 1);
      assert_eq(s[1], 2);
      assert_eq(s[2], 3);

      s[0] = 4;
      assert_eq((^vec)[0], 4);
    }

    {
      auto s = vec.slice();
      assert_eq(s[0], 4);
      assert_eq(s[1], 2);
      assert_eq(s[2], 3);
    }
  }

  {
    int x = 1;
    {
      std2::vector<int^> vec = {};
      vec^.push_back(^x);
      assert_eq(vec.size(), 1u);

      {
        const [int^; dyn]^ elems = vec.slice();
        assert_eq(*elems[0], 1);
      }

      [int^; dyn]^ elems = (^vec).slice();
      *elems[0] = 20;
    }
    assert_eq(x, 20);
  }

  {
    int x = 1;
    {
      int^ p = ^x;
      std2::vector<int^^> vec = {};
      vec^.push_back(^p);
      assert_eq(vec.size(), 1u);

      int^ const^ q = vec.slice()[0];
      (void)q;

      assert_eq(**q, 1);
    }
  }
}

template<std2::iterator T>
void check_definition(const T^ t)
{
  using item_type =
    typename impl<std2::slice_iterator<const int>, std2::iterator>::item_type;
}

void vector_iterator()
{
  {
    std2::vector<int> v = {};
    auto it = v.iter();

    check_definition(^it);

    auto m_val = (^it).std2::iterator::next();
    bool b = match (m_val) -> bool {
      .none => true;
      .some(x) => false;
    };
    assert(v.empty());
    assert(b);

    (^v).push_back(1);
    (^v).push_back(2);
    (^v).push_back(3);
    (^v).push_back(4);
    (^v).push_back(5);

    assert_eq(v.size(), 5u);

    int sum = 0;
    for (int x : v.iter()) {
      sum += x;
    }

    assert_eq(sum, 1 + 2 + 3 + 4 + 5);
  }
}

int main()
{
  vector_constructor();
  vector_iterator();
}
