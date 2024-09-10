#feature on safety

#include <cstring>
#include <type_traits>
#include <cstdio>

namespace std2
{

template<class T+>
class
[[safety::niche_zero, unsafe::send(T~is_send), unsafe::sync(T~is_sync)]]
box
{
  T* unsafe p_;
  T __phantom_data;

public:
  box(T* p) noexcept
    : p_(p)
  {
  }

  explicit
  box(T t) safe : p_(new T(rel t)) { }

  [[unsafe::drop_only(T)]]
  ~box() safe {
    delete p_;
  }

  template<typename... Ts+>
  static box make(Ts... args) safe {
    unsafe { return box(new T(rel args...)); }
  }

  T^ borrow(self^) noexcept safe {
    return ^*self->p_;
  }

  T const^ borrow(self const^) noexcept safe {
    return ^*self->p_;
  }

  T^ operator*(self^) noexcept safe {
    return self.borrow();
  }

  const T^ operator*(const self^) noexcept safe {
    return self.borrow();
  }

  T^ operator->(self^) noexcept safe {
    return ^*self->p_;
  }

  const T^ operator->(self const^) noexcept safe {
    return ^*self->p_;
  }

  T* get(self const^) noexcept safe {
    return self->p_;
  }

  T* leak(self) noexcept safe {
    auto p = self.p_;
    forget(rel self);
    return p;
  }

  T into_inner(self) noexcept safe {
    unsafe { T t = __rel_read(self.p_); }
    unsafe { ::operator delete(self.p_); }
    return rel t;
  }
};

}

struct dangling/(a)
{
  int const^/a x_;

  dangling(int const^/a x) safe : x_(x) {}
};

int main()
{
  std2::box<dangling> ps;
  {
    int x = 1234;
    ps = std2::box<dangling>::make(^const x);
  }
  *ps;
}
