#pragma once
#feature on safety

namespace std2
{

template<class T+>
choice optional
{
  default none,
  [[safety::unwrap]] some(T),
};

} // namespace std2
