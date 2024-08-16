#pragma once
#feature on safety

#include <cstddef>

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
