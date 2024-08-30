// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/slice.h>
#include <std2/string_view.h>
#include <std2/string_constant.h>

#include <cstddef>
#include <cstring>

namespace std2
{

template<class CharT>
class basic_string;

using string    = basic_string<char>;
using wstring   = basic_string<wchar_t>;
using u8string  = basic_string<char8_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;

template<class CharT>
class [[unsafe::send, unsafe::sync]]
  __attribute__((preferred_name(string))) 
  __attribute__((preferred_name(wstring)))
  __attribute__((preferred_name(u8string)))
  __attribute__((preferred_name(u16string)))
  __attribute__((preferred_name(u32string)))
basic_string
{
  public:

  using value_type = CharT;
  using size_type = std::size_t;

  static_assert(value_type~is_trivially_destructible);

  basic_string() safe
    : p_(nullptr)
    , size_{0}
    , capacity_{0}
  {
  }

  basic_string(string_constant<value_type> sc) safe
    : basic_string(basic_string_view<value_type>(sc))
  {
  }

  basic_string(basic_string_view<value_type> sv) safe
    : basic_string()
  {
    if (sv.empty()) return;

    auto n = sv.size() * sizeof(value_type);
    unsafe { p_ = static_cast<value_type*>(::operator new(n)); }
    unsafe { std::memcpy(p_, sv.data(), n); }
    size_ = sv.size();
    capacity_ = sv.size();
  }

  explicit
  basic_string(const [value_type; dyn]^ init) safe
    : basic_string(basic_string_view<value_type>(init))
  {
  }

  basic_string(basic_string const^ rhs) safe :
    basic_string(rhs.str())
  {
  }

  ~basic_string() safe {
    if (p_)
      unsafe { operator delete(p_); }
  }

  const [value_type; dyn]^ slice(self const^) noexcept safe {
    unsafe { return slice_from_raw_parts(self.data(), self.size()); }
  }

  basic_string_view<value_type> str(self const^) noexcept safe {
    using no_utf_check = typename basic_string_view<value_type>::no_utf_check;
    unsafe { return basic_string_view<value_type>(self.slice(), no_utf_check{}); }
  }

  operator basic_string_view<value_type>(self const^) noexcept safe {
    return self.str();
  }

  value_type const* data(self const^) noexcept safe {
    return self->p_;
  }

  size_type size(self const^) noexcept safe {
    return self->size_;
  }

  size_type capacity(self const^) noexcept safe {
    return self->capacity_;
  }

  void append(self^, basic_string_view<value_type> rhs) safe {
    if (auto len = self.size() + rhs.size(); len > self.capacity()) {
      unsafe { auto p = static_cast<value_type*>(::operator new(len * sizeof(value_type))); }
      unsafe { std::memcpy(p, self.data(), self.size() * sizeof(value_type)); }
      unsafe { ::operator delete(self->p_); }

      self->p_ = p;
      self->capacity_ = len;
    }

    unsafe { std::memcpy(self->p_ + self.size(), rhs.data(), rhs.size() * sizeof(value_type)); };
    self->size_ += rhs.size();
  }

  basic_string operator+(self const^, basic_string_view<value_type> rhs) safe {
    basic_string s = cpy self;
    mut s.append(rhs);
    drp self;
    return rel s;
  }

  private:
  value_type* unsafe p_;
  size_type size_;
  size_type capacity_;
};

} // namespace std2
