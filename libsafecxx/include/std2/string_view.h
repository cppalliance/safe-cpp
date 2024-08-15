// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/string_constant.h>
#include <std2/source_location.h>
#include <std2/__panic/codes.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

namespace std2
{
namespace detail
{

inline
std::size_t verify_utf/(a)(const [char; dyn]^/a str) noexcept safe
{
  auto const len = (*str)~length;
  std::size_t idx = 0;

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

    return std::size_t(-1);
  }
  return idx;
}

[[noreturn, safety::panic(panic_code::generic)]]
inline
void panic_impl(string_constant<char> msg, source_location loc = source_location::current()) safe
{
#if !defined(LIBSAFECXX_PANIC_THROWS)
  const [char; dyn]^ text = msg.text();
  unsafe __assert_fail(
    std::string((*text)~as_pointer, (*text)~length).c_str(),
    loc.file_name(),
    loc.line(),
    loc.function_name()
  );
#endif

  // TODO: without this, Circle claims this function _does_ return and refuses to compile
  throw "malformed utf";
}

} // namespace detail

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

public:
  basic_string_view() = delete;

  basic_string_view(string_constant<value_type> sc) noexcept safe
    : p_(sc.text())
  {
  }

  // TODO: forgetting `/a` will cause this code to compile when it shouldn't
  basic_string_view(const [value_type; dyn]^/a str) safe
    : p_(str)
  {
    auto pos = detail::verify_utf(str);
    if (pos != (*str)~length) detail::panic_impl("invalid utf detected");
  }

  value_type const* data(self) noexcept safe {
    return (*self.p_)~as_pointer;
  }

  size_type size(self) noexcept safe {
      return (*self.p_)~length;
  }

  bool operator==(self, basic_string_view rhs) noexcept safe {
    if(self.size() != rhs.size()) {
      return false;
    }
    unsafe return !std::memcmp(self.data(), rhs.data(), sizeof(value_type) * self.size());
  }

private:
  const [value_type; dyn]^/a p_;
};

} // namespace std2
