#feature on safety

template<class T+>
T replace/(a)(T^/a dst, T src) safe
{
  unsafe {
    T result = __rel_read(addr *dst);
    __rel_write(addr *dst, rel src);
    return result;
  }
}

template<class F, class R, class ...Args>
concept FnMut = requires(F f, Args ...args)
{
  requires safe(mut f(^args...));
};

template<class T+>
choice optional
{
  default none,
  [[safety::unwrap]] some(T);

  T unwrap(self) noexcept safe {
    return match(rel self) -> T {
      .some(t) => rel t;
      .none    => throw "invalid";
    };
  }

  template<class P>
  optional<T> take_if(self^, P p) safe
  requires FnMut<P, bool, T>
  {
    return match(*self) -> optional<T> {
      .some(^x) => return (
        p(x) ? replace<optional<T>>(self, .none) : optional<T>::none
      );
      .none => return .none;
    };
  }

  bool is_some(self const^) noexcept safe {
    return match(*self) {
      .some(_) => true;
      .none => false;
    };
  }

  bool is_none(self const^) noexcept safe {
    return !self.is_some();
  }
};

int main() safe
{
  struct C
  {
    static
    bool invoke/(a)(int^/a x) safe {
      bool b = (*x < 4321);
      return b;
    }
  };

  optional<int> opt = .some(1234);
  auto m_p = mut opt.take_if(addr C::invoke);

  if(!m_p.is_some()) throw 1234;
}
