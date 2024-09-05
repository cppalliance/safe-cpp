// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/slice.h>
#include <cstddef>
#include <memory>

namespace std2
{

template<class T+>
class initializer_list/(a) {
  // Point to byte data on the stack.
  T* unsafe _cur;
  T* _end;
  T^/a __phantom_data;

  explicit
  initializer_list([T; dyn]^/a data) noexcept safe :
    _cur((*data)~as_pointer),
    unsafe _end((*data)~as_pointer + (*data)~length) { }

public:
  // compiler-provided
  // must live in namespace std2

  initializer_list() noexcept safe : _cur(nullptr), _end(nullptr) { }
  initializer_list(const initializer_list&) = delete;
  initializer_list& operator=(const initializer_list&) = delete;

  ~initializer_list() safe requires(T~is_trivially_destructible) = default;

  [[unsafe::drop_only(T)]] 
  ~initializer_list() safe requires(!T~is_trivially_destructible) {
    std::destroy_n(_cur, _end - _cur);
  }

  [T; dyn]^ slice(self^) noexcept safe {
    return slice_from_raw_parts(self->_cur, self->size());
  }

  const [T; dyn]^ slice(const self^) noexcept safe {
    return slice_from_raw_parts(self->_cur, self->size());
  }

  optional<T> next(self^) noexcept safe {
    if(self->_cur != self->_end)
      return .some(__rel_read(self->_cur++));
    else
      return .none;
  }

  T* data(self^) noexcept safe {
    return self->_cur;
  }

  const T* data(const self^) noexcept safe {
    return self->_cur;
  }

  std::size_t size(const self^) noexcept safe {
    return (std::size_t)(self->_end - self->_cur);
  }

  // Unsafe call to advance. Use this after relocating data out of
  // data().
  void advance(self^, std::size_t size) noexcept {
    self->_cur += static_cast<std::ptrdiff_t>(size);
  }
};

} // namespace std2
