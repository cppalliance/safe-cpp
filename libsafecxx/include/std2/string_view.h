// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

/*

  basic_string_view synopsis

namespace std2
{

template<class CharT>
class basic_string_view/(a);

// C++-style typedefs.
using string_view    = basic_string_view<char>;
using wstring_view   = basic_string_view<wchar_t>;
using u8string_view  = basic_string_view<char8_t>;
using u16string_view = basic_string_view<char16_t>;
using u32string_view = basic_string_view<char32_t>;

// Rust-style typedefs.
using str    = basic_string_view<char>;
using wstr   = basic_string_view<wchar_t>;
using u8str  = basic_string_view<char8_t>;
using u16str = basic_string_view<char16_t>;
using u32str = basic_string_view<char32_t>;

template<class CharT>
class basic_string_view/(a)
{
public:
  using value_type             = CharT;
  using pointer                = value_type*;
  using const_pointer          = const value_type*;
  using reference              = value_type&;
  using const_reference        = const value_type&;
  // using const_iterator         = implementation-defined; // see [string.view.iterators]
  // using iterator               = const_iterator;201
  // using const_reverse_iterator = reverse_iterator<const_iterator>;
  // using reverse_iterator       = const_reverse_iterator;
  using size_type              = std::size_t;
  using difference_type        = std::ptrdiff_t;
  static constexpr size_type npos = size_type(-1);

  basic_string_view() = delete;

  basic_string_view(string_constant<value_type> sc) noexcept safe;

  basic_string_view(const [value_type; dyn]^/a str) safe;

  value_type const* data(self) noexcept safe;

  size_type size(self) noexcept safe;

  bool operator==(self, basic_string_view rhs) noexcept safe;

  const [value_type; dyn]^/a slice(self) noexcept safe;
};

} // namespace std2

*/

#include <std2/string_constant.h>
#include <std2/source_location.h>
#include <std2/slice.h>
#include <std2/__panic/codes.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

namespace std2
{

template<class CharT>
class basic_string_view/(a);

// C++-style typedefs.
using string_view    = basic_string_view<char>;
using wstring_view   = basic_string_view<wchar_t>;
using u8string_view  = basic_string_view<char8_t>;
using u16string_view = basic_string_view<char16_t>;
using u32string_view = basic_string_view<char32_t>;

// Rust-style typedefs.
using str    = basic_string_view<char>;
using wstr   = basic_string_view<wchar_t>;
using u8str  = basic_string_view<char8_t>;
using u16str = basic_string_view<char16_t>;
using u32str = basic_string_view<char32_t>;

template<class CharT>
class 
  __attribute__((preferred_name(string_view)))
  __attribute__((preferred_name(wstring_view)))
  __attribute__((preferred_name(u8string_view)))
  __attribute__((preferred_name(u16string_view)))
  __attribute__((preferred_name(u32string_view)))
basic_string_view/(a)
{
public:
  using value_type             = CharT;
  using pointer                = value_type*;
  using const_pointer          = const value_type*;
  using reference              = value_type&;
  using const_reference        = const value_type&;
  // using const_iterator         = implementation-defined; // see [string.view.iterators]
  // using iterator               = const_iterator;201
  // using const_reverse_iterator = reverse_iterator<const_iterator>;
  // using reverse_iterator       = const_reverse_iterator;
  using size_type              = std::size_t;
  using difference_type        = std::ptrdiff_t;
  static constexpr size_type npos = size_type(-1);

private:
  [[noreturn, safety::panic(panic_code::generic)]]
  static
  void panic_impl(string_constant<char> msg, source_location loc = source_location::current()) safe
  {
  #if !defined(LIBSAFECXX_PANIC_THROWS)
    const [char; dyn]^ text = msg.text();
    unsafe { __assert_fail(
      std::string((*text)~as_pointer, (*text)~length).c_str(),
      loc.file_name(),
      loc.line(),
      loc.function_name()
    ); }
  #else
    throw "malformed utf";
  #endif
  }

  static
  size_type verify_utf(const [char; dyn]^/a str) noexcept safe
  {
    static_assert(sizeof(char) == sizeof(char8_t));

    auto const len = (*str)~length;
    size_type idx = 0;

    for ( ; idx < len; ) {
      auto const c1 = str[idx];

      if ((0x80 & c1) == 0) {
        // ascii byte
        ++idx;
        continue;
      }

      // 2 byte codepoint
      // leading byte: 0b110xxxxx
      if ((0xc0 == (c1 & 0xe0))) {
        if (len - idx < 2) return idx;

        // invalid continuation byte
        if (0x80 != (str[idx + 1] & 0xc0)) return idx;

        idx += 2;
        continue;
      }

      // 3 byte codepoint
      // leading byte: 0b1110xxxx
      if (0xe0 == (c1 & 0xf0)) {
        if (len - idx < 3) return idx;

        // invalid continuation byte
        if (0x80 != (str[idx + 1] & 0xc0)) return idx;

        // invalid continuation byte
        if (0x80 != (str[idx + 2] & 0xc0)) return idx;

        idx += 3;
        continue;
      }

      // 4 byte codepoint
      // leading byte: 0b11110xxx
      if (0xf0 == (c1 & 0xf8)) {
        if (len - idx < 4) return idx;

        // invalid continuation byte
        if (0x80 != (str[idx + 1] & 0xc0)) return idx;

        // invalid continuation byte
        if (0x80 != (str[idx + 2] & 0xc0)) return idx;

        // invalid continuation byte
        if (0x80 != (str[idx + 3] & 0xc0)) return idx;

        idx += 4;
        continue;
      }

      return npos;
    }
    return idx;
  }

  static
  size_type verify_utf(const [char8_t; dyn]^/a str) noexcept safe
  {
    unsafe {
      auto const^ s = slice_from_raw_parts(
        reinterpret_cast<char const*>((*str)~as_pointer),
        (*str)~length);
    }
    return verify_utf(s);
  }

  static
  size_type verify_utf(const [char16_t; dyn]^/a str) noexcept safe
  {
    size_type idx = 0;
    auto const len = (*str)~length;

    for( ; idx < len; ) {
      auto const c1 = str[idx];

      if (c1 < 0xd800 || c1 >= 0xe000) {
        ++idx;
        continue;
      }

      // leading code point
      if (0xd800 == (0xfc00 & c1)) {
        if (len - idx < 2) return idx;

        // trailing code point
        if(0xdc00 != (0xfc00 & str[idx + 1])) return idx;

        idx += 2;
        continue;
      }

      return npos;
    }

    return idx;
  }

  static
  size_type verify_utf(const [char32_t; dyn]^/a str) noexcept safe
  {
    size_type idx = 0;
    auto const len = (*str)~length;

    for ( ; idx < len; ) {
      auto const c1 = str[idx];
      if (c1 < 0xd800 || (c1 > 0xdfff && c1 <= 0x10ffff)) {
        ++idx;
        continue;
      }

      return npos;
    }

    return idx;
  }

  static
  size_type verify_utf(const [wchar_t; dyn]^/a str) noexcept safe
  {
    if constexpr (sizeof(wchar_t) == 2) {
      unsafe {
        auto const^ s = slice_from_raw_parts(
          reinterpret_cast<char16_t const*>((*str)~as_pointer),
          (*str)~length) ;
      }
      return verify_utf(s);
    } else {
      static_assert(sizeof(wchar_t) == 4);

      unsafe {
        auto const^ s = slice_from_raw_parts(
          reinterpret_cast<char32_t const*>((*str)~as_pointer),
          (*str)~length) ;
      }
      return verify_utf(s);
    }
  }

public:
  struct no_utf_check {};

  basic_string_view() = delete;

  basic_string_view(string_constant<value_type> sc) noexcept safe
    : p_(sc.text())
  {
  }

  basic_string_view(const [value_type; dyn]^/a str) safe
    : p_(str)
  {
    auto pos = verify_utf(p_);
    if (pos != (*str)~length) panic_impl("invalid utf detected");
  }

  basic_string_view(const [value_type; dyn]^/a str, no_utf_check) noexcept
    : p_(str)
  {
  }

  value_type const* data(self) noexcept safe {
    return (*self.p_)~as_pointer;
  }

  size_type size(self) noexcept safe {
      return (*self.p_)~length;
  }

  bool empty(self) noexcept safe {
    return (self.size() == 0);
  }

  bool operator==(self, basic_string_view rhs) noexcept safe {
    if(self.size() != rhs.size()) {
      return false;
    }
    unsafe { return !std::memcmp(self.data(), rhs.data(), sizeof(value_type) * self.size()); }
  }

  const [value_type; dyn]^/a slice(self) noexcept safe {
    return self.p_;
  }

private:
  const [value_type; dyn]^/a p_;
};

} // namespace std2
