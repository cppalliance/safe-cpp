// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2/vector.h>

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

      // TODO: should this compile?
      // this works in Rust:
      // let mut x = 1_i32;
      // let mut vec = Vec::<&mut i32>::new();
      //
      // vec.push(&mut x);
      //
      // let xs : &[&mut i32] = vec.as_slice();
      // let _rawr: &&mut i32 = &xs[0];
      // const [int^; dyn]^ elems = vec.slice();

      [int^; dyn]^ elems = (^vec).slice();
      *elems[0] = 20;
    }

    assert_eq(x, 20);
  }
}

int main()
{
  vector_constructor();
}
