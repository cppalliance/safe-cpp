// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#feature on safety

#include <std2.h>
#include "lightweight_test.h"

void string_view_constructor() safe
{
  std2::string_constant<char> sc = "hello, world!";
  std2::string_view sv = sc;
  REQUIRE_EQ(sv.size(), (*sc.text())~length);
  REQUIRE_EQ(sv.data(), (*sc.text())~as_pointer);
  REQUIRE(sv == sc);
  REQUIRE(!(sv != sc));
}

// Encodes ucs into the UTF-8 buffer at s. Returns the number of characters
// encoded. 0 indicates error.
const [char; dyn]^
to_utf8([char; 4]^ s, char32_t ucs) safe
{
  if(ucs <= 0x007f) {
    s[0] = (char)ucs;
    unsafe { return std2::slice_from_raw_parts((*s)~as_pointer, 1); }

  } else if(ucs <= 0x07ff) {
    s[0] = static_cast<char>(0xc0 | (ucs>> 6));
    s[1] = static_cast<char>(0x80 | (0x3f & ucs));
    unsafe { return std2::slice_from_raw_parts((*s)~as_pointer, 2); }

  } else if(ucs <= 0xffff) {
    s[0] = static_cast<char>(0xe0 | (ucs>> 12));
    s[1] = static_cast<char>(0x80 | (0x3f & (ucs>> 6)));
    s[2] = static_cast<char>(0x80 | (0x3f & ucs));
    unsafe { return std2::slice_from_raw_parts((*s)~as_pointer, 3); }

  } else if (ucs <= 0x10ffff) {
    s[0] = static_cast<char>(0xf0 | (ucs>> 18));
    s[1] = static_cast<char>(0x80 | (0x3f & (ucs>> 12)));
    s[2] = static_cast<char>(0x80 | (0x3f & (ucs>> 6)));
    s[3] = static_cast<char>(0x80 | (0x3f & ucs));
    unsafe { return std2::slice_from_raw_parts((*s)~as_pointer, 4); }
  }
  unsafe { return std2::slice_from_raw_parts((*s)~as_pointer, 0); }
}

const [char8_t; dyn]^
to_utf8([char8_t; 4]^ s, char32_t ucs) safe
{
  if(ucs <= 0x007f) {
    s[0] = (char8_t)ucs;
    unsafe { return std2::slice_from_raw_parts((*s)~as_pointer, 1); }

  } else if(ucs <= 0x07ff) {
    s[0] = static_cast<char8_t>(0xc0 | (ucs>> 6));
    s[1] = static_cast<char8_t>(0x80 | (0x3f & ucs));
    unsafe { return std2::slice_from_raw_parts((*s)~as_pointer, 2); }

  } else if(ucs <= 0xffff) {
    s[0] = static_cast<char8_t>(0xe0 | (ucs>> 12));
    s[1] = static_cast<char8_t>(0x80 | (0x3f & (ucs>> 6)));
    s[2] = static_cast<char8_t>(0x80 | (0x3f & ucs));
    unsafe { return std2::slice_from_raw_parts((*s)~as_pointer, 3); }

  } else if (ucs <= 0x10ffff) {
    s[0] = static_cast<char8_t>(0xf0 | (ucs>> 18));
    s[1] = static_cast<char8_t>(0x80 | (0x3f & (ucs>> 12)));
    s[2] = static_cast<char8_t>(0x80 | (0x3f & (ucs>> 6)));
    s[3] = static_cast<char8_t>(0x80 | (0x3f & ucs));
    unsafe { return std2::slice_from_raw_parts((*s)~as_pointer, 4); }
  }
  unsafe { return std2::slice_from_raw_parts((*s)~as_pointer, 0); }
}

const [char16_t; dyn]^
to_utf16([char16_t; 2]^ s, char32_t ucs) safe
{
  if (ucs <= 0xffff) {
    s[0] = static_cast<char16_t>(ucs);
    unsafe { return std2::slice_from_raw_parts((*s)~as_pointer, 1); }
  }

  if (ucs <= 0x10ffff) {
    ucs -= 0x10000;
    s[0] = static_cast<char16_t>(0xd800 + (ucs >> 10));
    s[1] = static_cast<char16_t>(0xdc00 + (ucs & 0x03ff));
    unsafe { return std2::slice_from_raw_parts((*s)~as_pointer, 2); }
  }

  unsafe { return std2::slice_from_raw_parts((*s)~as_pointer, 0); }
}

void string_view_slice_ordinary_utf8_constructor() safe
{
  // ascii
  {
    const [char; dyn]^ str = "rawr";
    std2::string_view sv = str;
    REQUIRE_EQ(sv.size(), 5u);
    REQUIRE_EQ(sv.data(), (*str)~as_pointer);
  }

  {
    // outside valid range
    REQUIRE_THROWS(([]() safe {
      char const str[] = { (char)0xff };
      std2::string_view sv = str;
      (void)sv;
    }));
  }

  // 2 byte code points
  {
    const [char; dyn]^ str = "£";
    std2::string_view sv = str;
    REQUIRE_EQ(sv.size(), 3u);
    REQUIRE_EQ(sv.data(), (*str)~as_pointer);
  }

  {
    char const str[] = { (char)0xcf, (char)0xbf };
    std2::string_view sv = str;
    REQUIRE_EQ(sv.size(), 2u);
    REQUIRE_EQ(sv.data(), str);
  }

  {
    // invalid lengths
    REQUIRE_THROWS(([]() safe {
      char const str[] = { (char)0xcf };
      std2::string_view sv = str;
      (void)sv;
    }));
  }

  {
    // invalid continuation
    REQUIRE_THROWS(([]() safe {
      char const str[] = { (char)0xcf, (char)0xcf };
      std2::string_view sv = str;
      (void)sv;
    }));
  }

  // 3 byte code points
  {
    const [char; dyn]^ str = "한";
    std2::string_view sv = str;
    REQUIRE_EQ(sv.size(), 4u);
    REQUIRE_EQ(sv.data(), (*str)~as_pointer);
  }

  {
    char const str[] = { 	(char)0xed, (char)0x95, (char)0x9c };
    std2::string_view sv = str;
    REQUIRE_EQ(sv.size(), 3u);
    REQUIRE_EQ(sv.data(), str);
  }

  {
    // invalid length
      REQUIRE_THROWS(([]() safe {
        char const str[] = { (char)0xed };
        std2::string_view sv = str;
        (void)sv;
      }));
  }

  {
    // invalid length
    REQUIRE_THROWS(([]() safe {
      char const str[] = { (char)0xed, (char)0x95 };
      std2::string_view sv = str;
      (void)sv;
    }));
  }

  {
    // invalid continuation
    REQUIRE_THROWS(([]() safe {
      char const str[] = { (char)0xed, (char)0x95, (char)0xcc };
      std2::string_view sv = str;
      (void)sv;
    }));
  }

  {
    // invalid continuation
    REQUIRE_THROWS(([]() safe {
      char const str[] = { 	(char)0xed, (char)0xc5, (char)0x9c };
      std2::string_view sv = str;
      (void)sv;
    }));
  }

  // 4 byte code points
  {
    const [char; dyn]^ str = "𐍈";
    std2::string_view sv = str;
    REQUIRE_EQ(sv.size(), 5u);
    REQUIRE_EQ(sv.data(), (*str)~as_pointer);
  }

  {
    char const str[] = { 	(char)0xf0, (char)0x90, (char)0x8d, (char)0x88, };
    std2::string_view sv = str;
    REQUIRE_EQ(sv.size(), 4u);
    REQUIRE_EQ(sv.data(), str);
  }

  {
    // invalid length
    REQUIRE_THROWS(([]() safe {
      char const str[] = { 	(char)0xf0};
      std2::string_view sv = str;
      (void)sv;
    }));
  }

  {
    // invalid length
    REQUIRE_THROWS(([]() safe {
      char const str[] = { 	(char)0xf0, (char)0x90, };
      std2::string_view sv = str;
      (void)sv;
    }));
  }

  {
    // invalid continuation
    REQUIRE_THROWS(([]() safe {
      char const str[] = { 	(char)0xf0, (char)0xc0, (char)0x8d, (char)0x88, };
      std2::string_view sv = str;
      (void)sv;
    }));
  }

  {
    // invalid continuation
    REQUIRE_THROWS(([]() safe {
      char const str[] = { 	(char)0xf0, (char)0x90, (char)0xcd, (char)0x88, };
      std2::string_view sv = str;
      (void)sv;
    }));
  }

  {
    // invalid continuation
    REQUIRE_THROWS(([]() safe {
      char const str[] = { 	(char)0xf0, (char)0x90, (char)0x8d, (char)0xc8, };
      std2::string_view sv = str;
      (void)sv;
    }));
  }

  // prove we can parse the entire utf space
  {
    for (char32_t i = 0; i <= 0x10ffff; ++i) {
      [char; 4] buf = {};
      auto str = to_utf8(^buf, i);
      REQUIRE((*str)~length > 0);

      std2::string_view sv = str;
      REQUIRE(sv.size() > 0u);
      REQUIRE_EQ(sv.data(), (*str)~as_pointer);
    }
  }

  {
    const [char; dyn]^ str = "$£Иह€한𐍈";
    std2::string_view sv = str;

    REQUIRE_EQ(sv.size(), 19u);
    REQUIRE_EQ(sv.data(), (*str)~as_pointer);
  }
}

void string_view_slice_utf8_constructor() safe
{
  // ascii
  {
    const [char8_t; dyn]^ str = u8"rawr";
    std2::u8string_view sv = str;
    REQUIRE_EQ(sv.size(), 5u);
    REQUIRE_EQ(sv.data(), (*str)~as_pointer);
  }
}

void string_view_slice_utf16_constructor() safe
{
  // ascii
  {
    const [char16_t; dyn]^ str = u"rawr";
    std2::u16string_view sv = str;
    REQUIRE_EQ(sv.size(), 5u);
    REQUIRE_EQ(sv.data(), (*str)~as_pointer);
  }

  {
    const char16_t str[] = { (char16_t)0xffff };
    std2::u16string_view sv = str;

    REQUIRE_EQ(sv.size(), 1u);
    REQUIRE_EQ(sv.data(), str);
  }

  {
    const char16_t str[] = { (char16_t)0xfffe };
    std2::u16string_view sv = str;

    REQUIRE_EQ(sv.size(), 1u);
    REQUIRE_EQ(sv.data(), str);
  }

{
    const char16_t str[] = { (char16_t)0xfeff };
    std2::u16string_view sv = str;

    REQUIRE_EQ(sv.size(), 1u);
    REQUIRE_EQ(sv.data(), str);
  }

  {
    const [char16_t; dyn]^ str = u"€";
    std2::u16string_view sv = str;
    REQUIRE_EQ(sv.size(), 2u);
    REQUIRE_EQ(sv.data(), (*str)~as_pointer);
  }

  {
    const [char16_t; dyn]^ str = u"𐐷";
    std2::u16string_view sv = str;
    REQUIRE_EQ(sv.size(), 3u);
    REQUIRE_EQ(sv.data(), (*str)~as_pointer);
  }

  {
    const char16_t str[] = { (char16_t)0xd801, (char16_t)0xdc37 };
    std2::u16string_view sv = str;
    REQUIRE_EQ(sv.size(), 2u);
    REQUIRE_EQ(sv.data(), str);
  }

  {
    // length error
    REQUIRE_THROWS(([]() safe {
      const char16_t str[] = { (char16_t)0xd801 };
      std2::u16string_view sv =  str;
      (void)sv;
    }));
  }

  {
    // invalid leading surrogate
    REQUIRE_THROWS(([]() safe {
      const char16_t str[] = { (char16_t)0xf801, (char16_t)0xdc37 };
      std2::u16string_view sv =  str;
      (void)sv;
    }));
  }

  {
    // invalid trailing surrogate
    REQUIRE_THROWS(([]() safe {
      const char16_t str[] = { (char16_t)0xd801, (char16_t)0xfc37 };
      std2::u16string_view sv =  str;
      (void)sv;
    }));
  }

  {
    const [char16_t; dyn]^ str = u"𤭢";
    std2::u16string_view sv = str;
    REQUIRE_EQ(sv.size(), 3u);
    REQUIRE_EQ(sv.data(), (*str)~as_pointer);
  }

  // prove we can parse the entire utf space
  {
    for (char32_t i = 0; i <= 0x10ffff; ++i) {
      if (i >= 0xd800 && i <= 0xdfff) continue;

      [char16_t; 2] buf = {};
      auto str = to_utf16(^buf, i);
      REQUIRE((*str)~length > 0);

      std2::u16string_view sv = str;
      REQUIRE(sv.size() > 0u);
      REQUIRE_EQ(sv.data(), (*str)~as_pointer);
    }
  }

  {
    const [char16_t; dyn]^ str = u"$€𐐷𤭢";
    std2::u16string_view sv = str;

    REQUIRE_EQ(sv.size(), 7u);
    REQUIRE_EQ(sv.data(), (*str)~as_pointer);
  }

  {
    REQUIRE_THROWS(([]() safe {
      const char16_t str[] = { (char16_t)0xd800 };
      std2::u16string_view sv =  str;
      (void)sv;
    }));
  }

    {
    REQUIRE_THROWS(([]() safe {
      const char16_t str[] = { (char16_t)0xdfff };
      std2::u16string_view sv =  str;
      (void)sv;
    }));
  }
}

void string_view_slice_utf32_constructor() safe
{
  // prove we can parse the entire utf space
  {
    for (char32_t i = 0; i <= 0x10ffff; ++i) {
      if (i >= 0xd800 && i <= 0xdfff) continue;

      const char32_t str[] = { i };

      std2::u32string_view sv = str;
      REQUIRE(sv.size() > 0u);
      REQUIRE_EQ(sv.data(), str);
    }
  }

  {
    REQUIRE_THROWS(([]() safe {
      const char32_t str[] = { (char32_t)0xd800 };
      std2::u32string_view sv =  str;
      (void)sv;
    }));
  }

    {
    REQUIRE_THROWS(([]() safe {
      const char32_t str[] = { (char32_t)0xdfff };
      std2::u32string_view sv =  str;
      (void)sv;
    }));
  }
}

void string_view_slice_wstring_constructor() safe
{
  // ascii
  {
    const [wchar_t; dyn]^ str = L"rawr";
    std2::wstring_view sv = str;
    REQUIRE_EQ(sv.size(), 5u);
    REQUIRE_EQ(sv.data(), (*str)~as_pointer);
  }

  {
    const [wchar_t; dyn]^ str = L"한";
    std2::wstring_view sv = str;
    REQUIRE(sv.size() > 0);
    REQUIRE_EQ(sv.data(), (*str)~as_pointer);
  }
}

void string_view_compare() safe
{
  {
    std2::string_constant<char> str = "£";

    std2::string_view sv1 = str;
    std2::string_view sv2 = str;

    REQUIRE(sv1 == sv2);
    REQUIRE(!(sv1 != sv2));
  }

  {
    std2::string_constant<char> str1 = "£";
    std2::string_constant<char> str2 = "rawr";

    std2::string_view sv1 = str1;
    std2::string_view sv2 = str2;

    REQUIRE(sv1 != sv2);
    REQUIRE(!(sv1 == sv2));
  }
}

void string_view_slice() safe
{
  {
    std2::string_constant<char> str = "£";
    std2::string_view sv = str;

    auto s = sv.slice();

    REQUIRE_EQ((*s)~length, sv.size());
    REQUIRE_EQ((*s)~as_pointer, sv.data());
  }
}

void literal_test() safe
{
  using namespace std2::literals::string_literals;

  {
    std2::string_view sv = "hello, world!"sv2;
    REQUIRE(sv == std2::string_view("hello, world!"));
  }

  {
    std2::u8string_view sv = u8"hello, world!"sv2;
    REQUIRE(sv == std2::u8string_view(u8"hello, world!"));
  }

  {
    std2::u16string_view sv = u"hello, world!"sv2;
    REQUIRE(sv == std2::u16string_view(u"hello, world!"));
  }

  {
    std2::u32string_view sv = U"hello, world!"sv2;
    REQUIRE(sv == std2::u32string_view(U"hello, world!"));
  }

  {
    std2::wstring_view sv = L"hello, world!"sv2;
    REQUIRE(sv == std2::wstring_view(L"hello, world!"));
  }
}

TEST_MAIN(
  string_view_constructor,
  string_view_slice_ordinary_utf8_constructor,
  string_view_slice_utf8_constructor,
  string_view_slice_utf16_constructor,
  string_view_slice_utf32_constructor,
  string_view_slice_wstring_constructor,
  string_view_compare,
  string_view_slice,
  literal_test
)
