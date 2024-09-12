// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>
#include <string>

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

      s[0] = 17;
      assert_eq((^vec)[0], 17);

      (^vec)[0] = 4;
      assert_eq(vec[0], 4);
    }

    {
      auto s = vec.slice();
      assert_eq(s[0], 4);
      assert_eq(s[1], 2);
      assert_eq(s[2], 3);
    }

    {
      const std2::vector<int>^ v = ^vec;
      const int^ x = v[0];
      assert_eq(*x, 4);
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

  {
    std2::vector<int> xs = { 1, 2, 3, 4, 5 };
    assert_eq(xs.size(), 5u);
    for (int i = 0; i < 5; ++i) {
      auto idx = static_cast<std::size_t>(i);
      assert_eq(xs[idx], i + 1);
    }
  }

  {
    std2::vector<std2::box<int>> xs =
      { std2::box<int>(1), std2::box<int>(2), std2::box<int>(3), std2::box<int>(4), std2::box<int>(5) };
    assert_eq(xs.size(), 5u);
    for (int i = 0; i < 5; ++i) {
      auto idx = static_cast<std::size_t>(i);
      assert_eq(*xs[idx], i + 1);
    }
  }
}

template<std2::iterator T>
void check_definition(const T^ t) safe
{
  using item_type =
    typename impl<std2::slice_iterator<const int>, std2::iterator>::item_type;
}

void vector_iterator() safe
{
  {
    std2::vector<int> v = {};
    // v.data()[0];
    auto it = v.iter();

    check_definition(^it);

    auto m_val = mut it.std2::iterator::next();
    bool b = match (m_val) -> bool {
      .none => true;
      .some(x) => false;
    };
    assert_true(v.empty());
    assert_true(b);

    v^.push_back(1);
    v^.push_back(2);
    v^.push_back(3);
    v^.push_back(4);
    v^.push_back(5);

    assert_eq(v.size(), 5u);

    int sum = 0;
    for (int x : v.iter()) {
      sum += x;
    }

    assert_eq(sum, 1 + 2 + 3 + 4 + 5);
  }
}

void vector_string_view() safe
{
  unsafe { std::string s1 = "hello, world!"; }
  unsafe { std::string s2 = "walking home in the moonlight"; }
  unsafe { std::string s3 = "catching glimpses of my past life"; }

  unsafe { std2::string_view sv1 = std2::slice_from_raw_parts(s1.data(), s1.size()); }
  unsafe { std2::string_view sv2 = std2::slice_from_raw_parts(s2.data(), s2.size()); }
  unsafe { std2::string_view sv3 = std2::slice_from_raw_parts(s3.data(), s3.size()); }

  std2::vector<std2::string_view> strs = {};
  strs^.push_back(sv1);
  strs^.push_back(sv2);
  strs^.push_back(sv3);

  assert_eq(strs.size(), 3u);

  const std2::vector<std2::string_view>^ v = ^strs;
  const std2::string_view^ sv = v[0];
  assert_eq(sv, sv1);
}

void vector_box() safe
{
  std2::vector<std2::box<int>> xs = {};

  for (int i = 0; i < 16; ++i) {
    xs^.push_back(std2::box(1));
  }

  assert_eq(xs.size(), 16u);
}

void drop_only() safe
{
  {
    std2::vector<std2::string_view> p;
    {
      std2::string s("hello, world!");
      p = {s.str()};
      assert_true(p[0] == "hello, world!"sv2);
    }
  }
}


int main()
{
  vector_constructor();
  vector_iterator();
  vector_string_view();
  vector_box();
  drop_only();
}
