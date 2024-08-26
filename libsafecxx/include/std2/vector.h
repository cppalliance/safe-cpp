// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/iterator.h>
#include <std2/panic.h>
#include <std2/slice.h>
#include <std2/initializer_list.h>
#include <std2/optional.h>

#include <cstddef>
#include <cstring>
#include <memory>

#include <cstdio>

namespace std2
{

template<class T+>
class vector;

template<class T+>
class into_iterator
{
  using value_type = T;

  friend class vector<T>;

  value_type* unsafe origin_;
  value_type* unsafe p_;
  value_type* end_;


  public:
  into_iterator(value_type* p, value_type* end)
    : origin_(p)
    , p_(p)
    , end_(end)
  {
  }

  ~into_iterator() safe {
    while (p_ < end_) {
      unsafe { auto t = __rel_read(p_++); }
      (void)t;
    }
    unsafe { ::operator delete(origin_); }
  }

  optional<value_type> next(self^) noexcept safe {
    if (self->p_ < self->end_) {
      unsafe { return .some(__rel_read(self->p_++)); }
    } else {
      return .none;
    }
  }
};

// TODO: make vector conditionally Send/Sync
template<class T+>
class vector
{
public:
  using value_type = T;
  using size_type = std::size_t;

  vector() safe
    : p_(nullptr)
    , capacity_{0}
    , size_{0}
  {
  }

  vector(initializer_list<value_type> unsafe ilist) safe
    : vector()
  {
    self^.reserve(ilist.size());
    unsafe { relocate_array(self^.data(), ilist.data(), ilist.size()); }
    self.size_ = ilist.size();

    ilist^.advance(ilist.size());
  }

  ~vector() safe {
    // TODO: std::destroy_n() doesn't seem to like `int^` as a value_type
    // eventually we should fix this

    unsafe {
      auto const* end = self.data() + self.size();
      auto* pos = self^.data();

      while (pos < end) {
        auto t = __rel_read(pos);
        drp t;
        ++pos;
      }

      ::operator delete(p_);
    }
  }

  slice_iterator<const value_type> iter(const self^) noexcept safe {
    return slice_iterator<const value_type>(self.slice());
  }

  slice_iterator<value_type> iter(self^) noexcept safe {
    return slice_iterator<value_type>(self.slice());
  }

  value_type* data(self^) noexcept safe {
    return self->p_;
  }

  const value_type* data(const self^) noexcept safe {
    return self->p_;
  }

  size_type size(const self^) noexcept safe {
    return self->size_;
  }

  size_type capacity(const self^) noexcept safe {
    return self->capacity_;
  }

  bool empty(const self^) noexcept safe {
    return self.size() == 0;
  }

  void push_back(self^, T t) safe {
    if (self.capacity() == self.size()) { self.grow(); }

    unsafe { __rel_write(self->p_ + self->size_, rel t); }
   ++self->size_;
  }

  [value_type; dyn]^ slice(self^) noexcept safe {
    unsafe { return slice_from_raw_parts(self.data(), self.size()); }
  }

  const [value_type; dyn]^ slice(const self^) noexcept safe {
    unsafe { return slice_from_raw_parts(self.data(), self.size()); }
  }

  value_type^ operator[](self^, size_type i) noexcept safe {
    if (i >= self.size()) panic_bounds("vector subscript is out-of-bounds");
    unsafe { return ^self.data()[i]; }
  }

  const value_type^ operator[](const self^, size_type i) noexcept safe {
    if (i >= self.size()) panic_bounds("vector subscript is out-of-bounds");
    unsafe { return ^self.data()[i]; }
  }

  void reserve(self^, size_type n) safe {
    if (n <= self.capacity()) return;

    value_type* p;
    unsafe {
      p = static_cast<value_type*>(::operator new(n * sizeof(value_type)));
      relocate_array(p, self.data(), self.size());
      ::operator delete(self->p_);
    }

    self->p_ = p;
    self->capacity_ = n;
  }

private:

  static
  void relocate_array(value_type* dst, value_type const* src, size_type n) {
    // TODO: we should add a relocation check here
    // this code likely isn't sound for types with non-trivial/non-defaulted
    // relocation operators
    std::memcpy(dst, src, n * sizeof(value_type));
  }

  void grow(self^) safe {
    size_type cap = self.capacity();
    size_type ncap = cap ? 2 * cap : 1;
    self.reserve(ncap);
  }

  value_type* unsafe  p_;
  size_type capacity_;
  size_type size_;
  // value_type __phantom_data;
};

template<class T>
impl into_iterator<T>: iterator
{
  using item_type = T;

  optional<item_type> next(self^) safe override {
    return self.next();
  }
};

template<class T>
impl vector<T>: make_iter {
  using iter_type = slice_iterator<T const>;
  using iter_mut_type = slice_iterator<T>;
  using into_iter_type  = into_iterator<T>;

  iter_type iter(self const^) noexcept safe override {
    return slice_iterator<const T>(self->slice());
  }

  iter_mut_type iter(self^) noexcept safe override {
    return slice_iterator<T>(self^->slice());
  }

  into_iter_type iter(self) noexcept safe override {
    auto p = self^.data();
    auto len = self.size();
    forget(rel self);
    unsafe { return into_iter_type(p, p + len); }
  }
};

} // namespace std2
