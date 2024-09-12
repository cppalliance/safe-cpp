#feature on safety

template<class T+>
class manually_drop
{
  T t_;

public:
  explicit manually_drop(T t) noexcept safe;
  ~manually_drop() = trivial;

  void destroy(self^) noexcept;
  T const^ get(self const^) noexcept safe;
};

template<typename T+>
struct rc_inner
{
  manually_drop<T> data_;
  std::size_t strong_;
  std::size_t weak_;

  explicit
  rc_inner(T data) noexcept safe;
};

template<class T+>
class [[unsafe::send(false)]] rc
{
  rc_inner<T>* unsafe p_;

public:

  explicit rc(T t) safe;
  rc(rc const^ rhs) safe;
  [[unsafe::drop_only(T)]] ~rc() safe;

  T const^ operator*(self const^) noexcept safe {
    return *self->p_->data_.get();
  }
};

template<class T, class U>
void assert_eq(const T^ t, const U^ u) safe
{
  if (*t != *u) throw "unequal values";
}

void drop_only() safe
{
  {
    rc<int^> p;

    {
      int x = 1234;
      p = rc<int^>(^x);
      assert_eq(**p, 1234);
    }
  }
}

int main() safe
{
  drop_only();
}
