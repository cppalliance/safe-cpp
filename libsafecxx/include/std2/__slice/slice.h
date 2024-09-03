// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

namespace std2
{

template<class T>
auto slice_from_raw_parts/(a)(const T* p, std::size_t n) -> const [T; dyn]^/a {
  return ^*__slice_pointer(p, n);
}

template<class T>
auto slice_from_raw_parts/(a)(T* p, std::size_t n) -> [T; dyn]^/a {
  return ^*__slice_pointer(p, n);
}


} // namespace std2
