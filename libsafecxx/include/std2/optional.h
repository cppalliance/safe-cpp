#pragma once
#feature on safety

#include <std2/expected.h>
#include <std2/panic.h>

namespace std2
{

template<class T+>
choice optional
{
  default none,
  [[safety::unwrap]] some(T);

  template<class E+>
  expected<T, E> ok_or(self, E e) noexcept safe {
    return match(self) -> expected<T, E> {
      .some(t) => .ok(rel t);
      .none => .err(rel e);
    };
  }

  T expect(self, str msg) noexcept safe {
    return match(self) -> T {
      .some(t) => rel t;
      .none => panic(msg);
    };
  }

  T unwrap(self) noexcept safe {
    return match(self) -> T {
      .some(t) => rel t;
      .none => panic("unwrapping invalid optional");
    };
  }
};

} // namespace std2
