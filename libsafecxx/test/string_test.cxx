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
    std2::string s("hello, world!");
    assert_eq(s.size(), 13u);
    assert_eq(s.capacity(), 13u);
    assert_true(s == std2::string_view("hello, world!"));
    assert_true(s != std2::string_view(""));
  }

  {
    std2::string s{"hello, world!"};
    assert_eq(s.size(), 13u);
    assert_eq(s.capacity(), 13u);
    assert_true(s == std2::string_view("hello, world!"));
    assert_true(s != std2::string_view(""));
  }

  {
    char const buf[] = "hello, world!";
    std2::string s{buf};
    assert_eq(s.size(), 14u); // null terminator
    assert_eq(s.capacity(), 14u);
    assert_true(s == std2::string_view(buf));
    assert_true(s != std2::string_view(""));
  }

  {
    char const buf[] = "hello, world!";
    const [char; dyn]^ p_buf = buf;
    std2::string s{p_buf};
    assert_eq(s.size(), 14u); // null terminator
    assert_eq(s.capacity(), 14u);
    assert_true(s == std2::string_view(p_buf));
    assert_true(s != std2::string_view(""));
  }

  {
    std2::string_view sv = "hello, world!";
    std2::string s(sv);
    assert_eq(s.size(), 13u);
    assert_eq(s.capacity(), 13u);
    assert_true(s == sv);
    assert_true(s != std2::string_view(""));
  }
}

void string_append() safe
{
  {
    std2::string_view sv1 = "if I only had the heart";
    std2::string_view sv2 = " to find out exactly who you are";

    std2::string s(sv1);
    mut s.append(sv2);

    assert_eq(s.size(), sv1.size() + sv2.size());
    assert_eq(s.capacity(), s.size());
    assert_eq(s, std2::string_view("if I only had the heart to find out exactly who you are"));
    assert_true(s != std2::string_view(""));
  }

  {
    std2::string_view sv1 = "if I only had the heart";
    std2::string_view sv2 = " to find out exactly who you are";

    std2::string s1(sv1);
    std2::string s2(sv2);

    std2::string s(s1 + s2);

    assert_eq(s.size(), sv1.size() + sv2.size());
    assert_eq(s.capacity(), s.size());
    assert_eq(s, std2::string_view("if I only had the heart to find out exactly who you are"));
    assert_true(s != std2::string_view(""));
  }
}

void literal_test() safe
{
  using namespace std2::literals::string_literals;

  {
    std2::string s = "hello, world!"s;
    assert_true(s == std2::string_view("hello, world!"));
  }

  {
    std2::u8string s = u8"hello, world!"s;
    assert_true(s == std2::u8string_view(u8"hello, world!"));
  }
  {
    std2::u16string s = u"hello, world!"s;
    assert_true(s == std2::u16string_view(u"hello, world!"));
  }
  {
    std2::u32string s = U"hello, world!"s;
    assert_true(s == std2::u32string_view(U"hello, world!"));
  }
  {
    std2::wstring s = L"hello, world!"s;
    assert_true(s == std2::wstring_view(L"hello, world!"));
  }

}

int main() safe
{
  string_constructor();
  string_append();
  literal_test();
}
