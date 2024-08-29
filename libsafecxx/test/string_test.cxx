// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>

#include "helpers.h"

void string_constructor() safe
{
  {
    std2::string s = {};
    assert_eq(s.size(), 0u);
    assert_eq(s.capacity(), 0u);
  }

  {
    std2::string s = "hello, world!";
    assert_eq(s.size(), 13u);
    assert_eq(s.capacity(), 13u);
    assert(s == std2::string_view("hello, world!"));
    assert(s != std2::string_view(""));
  }

  {
    std2::string s{"hello, world!"};
    assert_eq(s.size(), 13u);
    assert_eq(s.capacity(), 13u);
    assert(s == std2::string_view("hello, world!"));
    assert(s != std2::string_view(""));
  }

  {
    char const buf[] = "hello, world!";
    std2::string s{buf};
    assert_eq(s.size(), 14u); // null terminator
    assert_eq(s.capacity(), 14u);
    assert(s == std2::string_view(buf));
    assert(s != std2::string_view(""));
  }

  {
    char const buf[] = "hello, world!";
    const [char; dyn]^ p_buf = buf;
    std2::string s{p_buf};
    assert_eq(s.size(), 14u); // null terminator
    assert_eq(s.capacity(), 14u);
    assert(s == std2::string_view(p_buf));
    assert(s != std2::string_view(""));
  }

  {
    std2::string_view sv = "hello, world!";
    std2::string s = sv;
    assert_eq(s.size(), 13u);
    assert_eq(s.capacity(), 13u);
    assert(s == sv);
    assert(s != std2::string_view(""));
  }
}

void string_append() safe
{
  {
    std2::string_view sv1 = "if I only had the heart";
    std2::string_view sv2 = " to find out exactly who you are";

    std2::string s = sv1;
    mut s.append(sv2);

    assert_eq(s.size(), sv1.size() + sv2.size());
    assert_eq(s.capacity(), s.size());
    assert_eq(s, std2::string_view("if I only had the heart to find out exactly who you are"));
    assert(s != std2::string_view(""));
  }

  {
    std2::string_view sv1 = "if I only had the heart";
    std2::string_view sv2 = " to find out exactly who you are";

    std2::string s1 = sv1;
    std2::string s2 = sv2;

    std2::string s = s1 + s2;

    assert_eq(s.size(), sv1.size() + sv2.size());
    assert_eq(s.capacity(), s.size());
    assert_eq(s, std2::string_view("if I only had the heart to find out exactly who you are"));
    assert(s != std2::string_view(""));
  }
}

int main() safe
{
  string_constructor();
  string_append();
}
