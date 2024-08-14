// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/string_constant.hxx>

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace std2
{
namespace detail
{

inline void verify_utf(const char* str, std::size_t count)
{
  auto const* end = str + count;
  for (auto pos = str; pos < end;) {
    auto const c1 = *pos;
    if ((0x80 & c1) == 0) {
      // ascii byte
      ++pos;
      continue;
    }

    // 2 byte codepoint
    // leading byte: 0b110xxxxx
    if ((0xc0 == (c1 & 0xe0))) {
      if (end - pos < 2) throw "length error";

      // invalid continuation byte
      if (0x80 != (pos[1] & 0xc0)) throw "invalid continuation byte";

      pos += 2;
      continue;
    }

    // 3 byte codepoint
    // leading byte: 0b1110xxxx
    if (0xe0 == (c1 & 0xf0)) {
      if (end - pos < 3) throw "length error";

      // invalid continuation byte
      if (0x80 != (pos[1] & 0xc0)) throw "invalid continuation byte";

      // invalid continuation byte
      if (0x80 != (pos[2] & 0xc0)) throw "invalid continuation byte";

      pos += 3;
      continue;
    }

    // 4 byte codepoint
    // leading byte: 0b11110xxx
    if (0xf0 == (c1 & 0xf8)) {
      if (end - pos < 4) throw "length error";

      // invalid continuation byte
      if (0x80 != (pos[1] & 0xc0)) throw "invalid continuation byte";

      // invalid continuation byte
      if (0x80 != (pos[2] & 0xc0)) throw "invalid continuation byte";

      // invalid continuation byte
      if (0x80 != (pos[3] & 0xc0)) throw "invalid continuation byte";

      pos += 4;
      continue;
    }
    throw 1234;
  }
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

  basic_string_view() = delete;

  basic_string_view(string_constant<value_type> sc) noexcept safe
    : p_(sc.text())
  {
  }

  // TODO: forgetting `/a` will cause this code to compile when it shouldn't
  basic_string_view(const [value_type; dyn]^/a str) safe
    : p_(str)
  {
    unsafe detail::verify_utf((*self.p_)~as_pointer, (*self.p_)~length);
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
