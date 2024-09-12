#feature on safety

namespace std2 {

// unsafe_cell must be in std2 for the declaration to be seen by the
// compiler.
// We could improve by making an [[unsafe_cell]] attribute.
template<class T+>
class [[unsafe::sync(false)]] unsafe_cell
{
  T t_;

public:
  unsafe_cell() = default;

  explicit
  unsafe_cell(T t) noexcept safe
    : t_(rel t)
  {
  }

  T* get(self const^) noexcept safe {
    return const_cast<T*>(addr self->t_);
  }
};
}

template<class T+>
class [[unsafe::sync(false)]] cell
{
  std2::unsafe_cell<T> t_;

  public:

  explicit cell(T t) noexcept safe
    : t_(rel t)
  {
  }

  T get(self const^) safe {
    // rely on implicit copy operator erroring out for types with non-trivial
    // destructors or types that have user-defined copy constructors
    unsafe { return cpy *self->t_.get(); }
  }

  void set(self const^, T t) safe {
    unsafe { *self->t_.get() = rel t; }
  }
};

template<class T+>
class
[[safety::niche_zero, unsafe::send(T~is_send), unsafe::sync(T~is_sync)]]
box
{
  T* unsafe p_;
  T __phantom_data;

public:
  explicit
  box(T t) safe
    : p_(new T(rel t))
  {
  }

  [[unsafe::drop_only(T)]]
  ~box() safe {
    delete p_;
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
};

template<class T+>
choice optional
{
  default none,
  [[safety::unwrap]] some(T);
};

struct S/(a)
{
  cell<optional<S/a const^/a>> x_;
  box<int> p_;

  S(box<int> p) safe
    : x_(optional<S/a const^/a>(.none))
    , p_(rel p)
  {
  }

  ~S() safe {
  }
};

int main() safe
{
  // shouldn't compile
  {
    S s1(box<int>(1234));
    S s2(box<int>(4321));
    s1.x_.set(.some(^const s2));
    s2.x_.set(.some(^const s1));
  }
}
