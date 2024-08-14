// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/string_constant.hxx>

#include <cstddef>
#include <cstring>

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

  basic_string_view(string_constant<value_type> sc) noexcept safe
    : p_(sc.text())
  {
  }

  basic_string_view(const [value_type; dyn]^ str) noexcept safe
    : p_(str)
  {
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
