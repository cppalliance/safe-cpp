// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>

#include "lightweight_test.h"

void string_constructor() safe
{
  {
    std2::string s = {};
    REQUIRE_EQ(s.size(), 0u);
    REQUIRE_EQ(s.capacity(), 0u);
  }

  {
    std2::string s("hello, world!");
    REQUIRE_EQ(s.size(), 13u);
    REQUIRE_EQ(s.capacity(), 13u);
    REQUIRE(s == std2::string_view("hello, world!"));
    REQUIRE(s != std2::string_view(""));
  }

  {
    std2::string s{"hello, world!"};
    REQUIRE_EQ(s.size(), 13u);
    REQUIRE_EQ(s.capacity(), 13u);
    REQUIRE(s == std2::string_view("hello, world!"));
    REQUIRE(s != std2::string_view(""));
  }

  {
    char const buf[] = "hello, world!";
    std2::string s{buf};
    REQUIRE_EQ(s.size(), 14u); // null terminator
    REQUIRE_EQ(s.capacity(), 14u);
    REQUIRE(s == std2::string_view(buf));
    REQUIRE(s != std2::string_view(""));
  }

  {
    char const buf[] = "hello, world!";
    const [char; dyn]^ p_buf = buf;
    std2::string s{p_buf};
    REQUIRE_EQ(s.size(), 14u); // null terminator
    REQUIRE_EQ(s.capacity(), 14u);
    REQUIRE(s == std2::string_view(p_buf));
    REQUIRE(s != std2::string_view(""));
  }

  {
    std2::string_view sv = "hello, world!";
    std2::string s(sv);
    REQUIRE_EQ(s.size(), 13u);
    REQUIRE_EQ(s.capacity(), 13u);
    REQUIRE(s == sv);
    REQUIRE(s != std2::string_view(""));
  }
}

void string_append() safe
{
  {
    std2::string_view sv1 = "if I only had the heart";
    std2::string_view sv2 = " to find out exactly who you are";

    std2::string s(sv1);
    mut s.append(sv2);

    REQUIRE_EQ(s.size(), sv1.size() + sv2.size());
    REQUIRE_EQ(s.capacity(), s.size());
    REQUIRE_EQ(s, std2::string_view("if I only had the heart to find out exactly who you are"));
    REQUIRE(s != std2::string_view(""));
  }

  {
    std2::string_view sv1 = "if I only had the heart";
    std2::string_view sv2 = " to find out exactly who you are";

    std2::string s1(sv1);
    std2::string s2(sv2);

    std2::string s(s1 + s2);

    REQUIRE_EQ(s.size(), sv1.size() + sv2.size());
    REQUIRE_EQ(s.capacity(), s.size());
    REQUIRE_EQ(s, std2::string_view("if I only had the heart to find out exactly who you are"));
    REQUIRE(s != std2::string_view(""));
  }
}

void literal_test() safe
{
  using namespace std2::literals::string_literals;

  {
    std2::string s = "hello, world!"s2;
    REQUIRE(s == std2::string_view("hello, world!"));
  }

  {
    std2::u8string s = u8"hello, world!"s2;
    REQUIRE(s == std2::u8string_view(u8"hello, world!"));
  }
  {
    std2::u16string s = u"hello, world!"s2;
    REQUIRE(s == std2::u16string_view(u"hello, world!"));
  }
  {
    std2::u32string s = U"hello, world!"s2;
    REQUIRE(s == std2::u32string_view(U"hello, world!"));
  }
  {
    std2::wstring s = L"hello, world!"s2;
    REQUIRE(s == std2::wstring_view(L"hello, world!"));
  }

}

TEST_MAIN(
  string_constructor,
  string_append,
  literal_test
)
