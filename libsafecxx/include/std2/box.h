#pragma once
#feature on safety

namespace std2
{

template<class T+>
class box
{
  T* p_;
  T __phantom_data;

public:
  box(T* p) noexcept
    : p_(p)
  {
  }

  box(T t) safe
    : unsafe p_(static_cast<T*>(::operator new(sizeof(T))))
  {
    unsafe __rel_write(p_, rel t);
  }

  ~box() safe
  {
    unsafe ::operator delete(p_);
  }

  T^ operator*(self^) noexcept safe {
    unsafe return ^*self->p_;
  }

  const T^ operator*(const self^) noexcept safe {
    unsafe return ^*self->p_;
  }
};

} // namespace std2
