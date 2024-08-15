// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2/string_view.h>
#include <std2/string_constant.h>

template<class T, class U>
void assert_eq(const T^ t, const U^ u) safe
{
  if (*t != *u) throw "unequal values";
}

void assert(bool b) safe
{
  if (!b) throw "failed boolean assertion";
}

template<class F>
void assert_throws(F f) // safe
{
  bool threw = false;
  try {
    f();
  } catch(...) {
    threw = true;
  }
  assert(threw);
}

void string_view_constructor() safe
{
  // TODO: this segfaults, should eventually be fixed
  // std2::string_constant<char> sc("rawr");

  std2::string_constant<char> sc = "hello, world!";
  std2::string_view sv = sc;
  assert_eq(sv.size(), (*sc.text())~length);
  assert_eq(sv.data(), (*sc.text())~as_pointer);
  assert(sv == sc);
  assert(!(sv != sc));
}

// Encodes ucs into the UTF-8 buffer at s. Returns the number of characters
// encoded. 0 indicates error.
const [char; dyn]^
to_utf8([char; 4]^ s, char32_t ucs) safe
{
  if(ucs <= 0x007f) {
    s[0] = (char)ucs;
    unsafe return ^*__slice_pointer((*s)~as_pointer, 1);

  } else if(ucs <= 0x07ff) {
    s[0] = static_cast<char>(0xc0 | (ucs>> 6));
    s[1] = static_cast<char>(0x80 | (0x3f & ucs));
    unsafe return ^*__slice_pointer((*s)~as_pointer, 2);

  } else if(ucs <= 0xffff) {
    s[0] = static_cast<char>(0xe0 | (ucs>> 12));
    s[1] = static_cast<char>(0x80 | (0x3f & (ucs>> 6)));
    s[2] = static_cast<char>(0x80 | (0x3f & ucs));
    unsafe return ^*__slice_pointer((*s)~as_pointer, 3);

  } else if(ucs <= 0x10ffff) {
    s[0] = static_cast<char>(0xf0 | (ucs>> 18));
    s[1] = static_cast<char>(0x80 | (0x3f & (ucs>> 12)));
    s[2] = static_cast<char>(0x80 | (0x3f & (ucs>> 6)));
    s[3] = static_cast<char>(0x80 | (0x3f & ucs));
    unsafe return ^*__slice_pointer((*s)~as_pointer, 4);
  }
  unsafe return ^*__slice_pointer((*s)~as_pointer, 0);
}

void string_view_slice_utf8_constructor() safe
{
  // ascii
  {
    const [char; dyn]^ str = "rawr";
    std2::string_view sv = str;
    assert_eq(sv.size(), 5u);
    assert_eq(sv.data(), (*str)~as_pointer);
  }

  {
    // TODO: figure out how to get safe lambdas so we can make this safe too

    // outside valid range
    unsafe assert_throws([]() {
      char const str[] = { (char)0xff };
      std2::string_view sv = str;
      (void)sv;
    });
  }

  // 2 byte code points
  {
    const [char; dyn]^ str = "£";
    std2::string_view sv = str;
    assert_eq(sv.size(), 3u);
    assert_eq(sv.data(), (*str)~as_pointer);
  }

  {
    char const str[] = { (char)0xcf, (char)0xbf };
    std2::string_view sv = str;
    assert_eq(sv.size(), 2u);
    assert_eq(sv.data(), str);
  }

  {
    // TODO: figure out how to get safe lambdas so we can make this safe too

    // invalid lengths
    unsafe assert_throws([]() {
      char const str[] = { (char)0xcf };
      std2::string_view sv = str;
      (void)sv;
    });
  }

  {
    // TODO: figure out how to get safe lambdas so we can make this safe too

    // invalid continuation
    unsafe assert_throws([]() {
      char const str[] = { (char)0xcf, (char)0xcf };
      std2::string_view sv = str;
      (void)sv;
    });
  }

  // 3 byte code points
  {
    const [char; dyn]^ str = "한";
    std2::string_view sv = str;
    assert_eq(sv.size(), 4u);
    assert_eq(sv.data(), (*str)~as_pointer);
  }

  {
    char const str[] = { 	(char)0xed, (char)0x95, (char)0x9c };
    std2::string_view sv = str;
    assert_eq(sv.size(), 3u);
    assert_eq(sv.data(), str);
  }

  {
    // TODO: figure out how to get safe lambdas so we can make this safe too

    // invalid length
    unsafe assert_throws([]() {
      char const str[] = { (char)0xed };
      std2::string_view sv = str;
      (void)sv;
    });
  }

  {
    // TODO: figure out how to get safe lambdas so we can make this safe too

    // invalid length
    unsafe assert_throws([]() {
      char const str[] = { (char)0xed, (char)0x95 };
      std2::string_view sv = str;
      (void)sv;
    });
  }

  {
    // TODO: figure out how to get safe lambdas so we can make this safe too

    // invalid continuation
    unsafe assert_throws([]() {
      char const str[] = { (char)0xed, (char)0x95, (char)0xcc };
      std2::string_view sv = str;
      (void)sv;
    });
  }

  {
    // TODO: figure out how to get safe lambdas so we can make this safe too

    // invalid continuation
    unsafe assert_throws([]() {
      char const str[] = { 	(char)0xed, (char)0xc5, (char)0x9c };
      std2::string_view sv = str;
      (void)sv;
    });
  }

  // 4 byte code points
  {
    const [char; dyn]^ str = "𐍈";
    std2::string_view sv = str;
    assert_eq(sv.size(), 5u);
    assert_eq(sv.data(), (*str)~as_pointer);
  }

  {
    char const str[] = { 	(char)0xf0, (char)0x90, (char)0x8d, (char)0x88, };
    std2::string_view sv = str;
    assert_eq(sv.size(), 4u);
    assert_eq(sv.data(), str);
  }

  {
    // TODO: figure out how to get safe lambdas so we can make this safe too

    // invalid length
    unsafe assert_throws([]() {
      char const str[] = { 	(char)0xf0};
      std2::string_view sv = str;
      (void)sv;
    });
  }

  {
    // TODO: figure out how to get safe lambdas so we can make this safe too

    // invalid length
    unsafe assert_throws([]() {
      char const str[] = { 	(char)0xf0, (char)0x90, };
      std2::string_view sv = str;
      (void)sv;
    });
  }

  {
    // TODO: figure out how to get safe lambdas so we can make this safe too

    // invalid continuation
    unsafe assert_throws([]() {
      char const str[] = { 	(char)0xf0, (char)0xc0, (char)0x8d, (char)0x88, };
      std2::string_view sv = str;
      (void)sv;
    });
  }

  {
    // TODO: figure out how to get safe lambdas so we can make this safe too

    // invalid continuation
    unsafe assert_throws([]() {
      char const str[] = { 	(char)0xf0, (char)0x90, (char)0xcd, (char)0x88, };
      std2::string_view sv = str;
      (void)sv;
    });
  }

  {
    // TODO: figure out how to get safe lambdas so we can make this safe too

    // invalid continuation
    unsafe assert_throws([]() {
      char const str[] = { 	(char)0xf0, (char)0x90, (char)0x8d, (char)0xc8, };
      std2::string_view sv = str;
      (void)sv;
    });
  }

  // prove we can parse the entire utf space
  {
    for (char32_t i = 0; i < 0x10ffff; ++i) {
      [char; 4] buf = {};
      auto str = to_utf8(^buf, i);
      assert((*str)~length > 0);

      std2::string_view sv = str;
      assert(sv.size() > 0u);
      assert_eq(sv.data(), (*str)~as_pointer);
    }
  }

  {
    const [char; dyn]^ str = "$£Иह€한𐍈";
    std2::string_view sv = str;

    assert_eq(sv.size(), 19u);
    assert_eq(sv.data(), (*str)~as_pointer);
  }
}

int main()
{
  string_view_constructor();
  string_view_slice_utf8_constructor();
}
