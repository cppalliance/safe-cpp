#feature on safety

#include <cstdio>
#include <memory>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <new>
#include <cstring>
#include <atomic>
#include <string>

namespace std2 {

////////////////////////////////////////////////////////////////////////////////
// __panic/codes.h

// These must be coordinated with the compiler.
enum class panic_code {
  generic,
  bounds,
  divide_by_zero,
  lifetime,
};

////////////////////////////////////////////////////////////////////////////////
// __magic/magic.h

// N must be deduced by the implementation.
template<size_t N>
struct subarray_size { };

// Add to operator[] functions for faster calls in `unchecked` contexts.
struct no_runtime_check { };

////////////////////////////////////////////////////////////////////////////////
// tuple.h

template<typename... Ts+>
struct tuple {
  [[circle::no_unique_address_any]] Ts ...m;
};

////////////////////////////////////////////////////////////////////////////////
// string_constant.h

template<class CharT>
class string_constant
{
  const [CharT; dyn]^/static _text;

  // The compiler will provide this deleted constructor.
  // This class must live in namespace std2.
  string_constant() = delete;

public:
  const [CharT; dyn]^/static text(self) noexcept safe
  {
    return self._text;
  }
};

////////////////////////////////////////////////////////////////////////////////
// __slice/slice.h

template<class T>
auto slice_from_raw_parts/(a)(const T* p, std::size_t n) -> const [T; dyn]^/a {
  return ^*__slice_pointer(p, n);
}

template<class T>
auto slice_from_raw_parts/(a)(T* p, std::size_t n) -> [T; dyn]^/a {
  return ^*__slice_pointer(p, n);
}

////////////////////////////////////////////////////////////////////////////////
// source_location.h

class source_location
{
public:
  static constexpr
  source_location
  current(
    char const* file_name     = __builtin_FILE(),
    char const* function_name = __builtin_FUNCTION(),
    uint32_t    line          = __builtin_LINE(),
    uint32_t    column        = __builtin_COLUMN()) noexcept safe
  {
    source_location loc{};
    loc._file_name     = file_name;
    loc._function_name = function_name;
    loc._line          = line;
    loc._column        = column;
    return loc;
  }

  constexpr const char* file_name(const self^) noexcept safe {
    return self->_file_name;
  }

  constexpr const char* function_name(const self^) noexcept safe {
    return self->_function_name;
  }

  constexpr uint32_t line(const self^) noexcept safe {
    return self->_line;
  }

  constexpr uint32_t column(const self^) noexcept safe {
    return self->_column;
  }

private:
  char const* unsafe _file_name;
  char const* unsafe _function_name;
  uint32_t _line;
  uint32_t _column;
};

////////////////////////////////////////////////////////////////////////////////
// string_view.h

template<class CharT>
class basic_string_view/(a);

// C++-style typedefs.
using string_view    = basic_string_view<char>;
using wstring_view   = basic_string_view<wchar_t>;
using u8string_view  = basic_string_view<char8_t>;
using u16string_view = basic_string_view<char16_t>;
using u32string_view = basic_string_view<char32_t>;

// Rust-style typedefs.
using str    = basic_string_view<char>;
using wstr   = basic_string_view<wchar_t>;
using u8str  = basic_string_view<char8_t>;
using u16str = basic_string_view<char16_t>;
using u32str = basic_string_view<char32_t>;

template<class CharT>
class
  __attribute__((preferred_name(string_view)))
  __attribute__((preferred_name(wstring_view)))
  __attribute__((preferred_name(u8string_view)))
  __attribute__((preferred_name(u16string_view)))
  __attribute__((preferred_name(u32string_view)))
basic_string_view/(a)
{
public:
  using value_type             = CharT;
  using pointer                = value_type*;
  using const_pointer          = const value_type*;
  using reference              = value_type&;
  using const_reference        = const value_type&;
  // using const_iterator         = implementation-defined; // see [string.view.iterators]
  // using iterator               = const_iterator;201
  // using const_reverse_iterator = reverse_iterator<const_iterator>;
  // using reverse_iterator       = const_reverse_iterator;
  using size_type              = std::size_t;
  using difference_type        = std::ptrdiff_t;
  static constexpr size_type npos = size_type(-1);

private:
  [[noreturn, safety::panic(panic_code::generic)]]
  static
  void panic_impl(string_constant<char> msg, source_location loc = source_location::current()) safe
  {
  #if !defined(LIBSAFECXX_PANIC_THROWS)
    const [char; dyn]^ text = msg.text();

    unsafe {
      fprintf(stderr,
        "%s:%d:%d\n%s\n%.*s\n",
        loc.file_name(),
        loc.line(),
        loc.column(),
        loc.function_name(),
        (*text)~length,
        (*text)~as_pointer);
      fflush(stderr);
      abort();
    }
  #else
    throw "malformed utf";
  #endif
  }

  static
  size_type verify_utf(const [char; dyn]^/a str) noexcept safe
  {
    static_assert(sizeof(char) == sizeof(char8_t));

    auto const len = (*str)~length;
    size_type idx = 0;

    for ( ; idx < len; ) {
      auto const c1 = str[idx];

      if ((0x80 & c1) == 0) {
        // ascii byte
        ++idx;
        continue;
      }

      // 2 byte codepoint
      // leading byte: 0b110xxxxx
      if ((0xc0 == (c1 & 0xe0))) {
        if (len - idx < 2) return idx;

        // invalid continuation byte
        if (0x80 != (str[idx + 1] & 0xc0)) return idx;

        idx += 2;
        continue;
      }

      // 3 byte codepoint
      // leading byte: 0b1110xxxx
      if (0xe0 == (c1 & 0xf0)) {
        if (len - idx < 3) return idx;

        // invalid continuation byte
        if (0x80 != (str[idx + 1] & 0xc0)) return idx;

        // invalid continuation byte
        if (0x80 != (str[idx + 2] & 0xc0)) return idx;

        idx += 3;
        continue;
      }

      // 4 byte codepoint
      // leading byte: 0b11110xxx
      if (0xf0 == (c1 & 0xf8)) {
        if (len - idx < 4) return idx;

        // invalid continuation byte
        if (0x80 != (str[idx + 1] & 0xc0)) return idx;

        // invalid continuation byte
        if (0x80 != (str[idx + 2] & 0xc0)) return idx;

        // invalid continuation byte
        if (0x80 != (str[idx + 3] & 0xc0)) return idx;

        idx += 4;
        continue;
      }

      return npos;
    }
    return idx;
  }

  static
  size_type verify_utf(const [char8_t; dyn]^/a str) noexcept safe
  {
    unsafe {
      auto const^ s = slice_from_raw_parts(
        reinterpret_cast<char const*>((*str)~as_pointer),
        (*str)~length);
    }
    return verify_utf(s);
  }

  static
  size_type verify_utf(const [char16_t; dyn]^/a str) noexcept safe
  {
    size_type idx = 0;
    auto const len = (*str)~length;

    for( ; idx < len; ) {
      auto const c1 = str[idx];

      if (c1 < 0xd800 || c1 >= 0xe000) {
        ++idx;
        continue;
      }

      // leading code point
      if (0xd800 == (0xfc00 & c1)) {
        if (len - idx < 2) return idx;

        // trailing code point
        if(0xdc00 != (0xfc00 & str[idx + 1])) return idx;

        idx += 2;
        continue;
      }

      return npos;
    }

    return idx;
  }

  static
  size_type verify_utf(const [char32_t; dyn]^/a str) noexcept safe
  {
    size_type idx = 0;
    auto const len = (*str)~length;

    for ( ; idx < len; ) {
      auto const c1 = str[idx];
      if (c1 < 0xd800 || (c1 > 0xdfff && c1 <= 0x10ffff)) {
        ++idx;
        continue;
      }

      return npos;
    }

    return idx;
  }

  static
  size_type verify_utf(const [wchar_t; dyn]^/a str) noexcept safe
  {
    if constexpr (sizeof(wchar_t) == 2) {
      unsafe {
        auto const^ s = slice_from_raw_parts(
          reinterpret_cast<char16_t const*>((*str)~as_pointer),
          (*str)~length) ;
      }
      return verify_utf(s);
    } else {
      static_assert(sizeof(wchar_t) == 4);

      unsafe {
        auto const^ s = slice_from_raw_parts(
          reinterpret_cast<char32_t const*>((*str)~as_pointer),
          (*str)~length) ;
      }
      return verify_utf(s);
    }
  }

public:
  struct no_utf_check {};

  basic_string_view() = delete;

  basic_string_view(string_constant<value_type> sc) noexcept safe
    : p_(sc.text())
  {
  }

  basic_string_view(const [value_type; dyn]^/a str) safe
    : p_(str)
  {
    auto pos = verify_utf(p_);
    if (pos != (*str)~length) panic_impl("invalid utf detected");
  }

  basic_string_view(const [value_type; dyn]^/a str, no_utf_check) noexcept
    : p_(str)
  {
  }

  value_type const* data(self) noexcept safe {
    return (*self.p_)~as_pointer;
  }

  size_type size(self) noexcept safe {
      return (*self.p_)~length;
  }

  bool empty(self) noexcept safe {
    return (self.size() == 0);
  }

  bool operator==(self, basic_string_view rhs) noexcept safe {
    if(self.size() != rhs.size()) {
      return false;
    }
    unsafe { return !std::memcmp(self.data(), rhs.data(), sizeof(value_type) * self.size()); }
  }

  const [value_type; dyn]^/a slice(self) noexcept safe {
    return self.p_;
  }

private:
  const [value_type; dyn]^/a p_;
};

namespace literals
{
namespace string_literals
{

inline
auto operator""sv2(char const* p, std::size_t len) noexcept safe -> string_view/static
{
  unsafe { return string_view(slice_from_raw_parts(p, len)); }
}

inline
auto operator""sv2(char8_t const* p, std::size_t len) noexcept safe -> u8string_view/static
{
  unsafe { return u8string_view(slice_from_raw_parts(p, len)); }
}

inline
auto operator""sv2(char16_t const* p, std::size_t len) noexcept safe -> u16string_view/static
{
  unsafe { return u16string_view(slice_from_raw_parts(p, len)); }
}

inline
auto operator""sv2(char32_t const* p, std::size_t len) noexcept safe -> u32string_view/static
{
  unsafe { return u32string_view(slice_from_raw_parts(p, len)); }
}

inline
auto operator""sv2(wchar_t const* p, std::size_t len) noexcept safe -> wstring_view/static
{
  unsafe { return wstring_view(slice_from_raw_parts(p, len)); }
}

} // namespace string_literals
} // namespace literals


////////////////////////////////////////////////////////////////////////////////
// panic.h

// Abort the program
// Panic functions are categorized and marked with an safety::panic(N) attribute.
// This makes it easy for the frontend to toggle on or off panic calls on a
// per-file basis.
[[noreturn, safety::panic(panic_code::generic)]]
inline void panic(
  str msg, source_location loc = source_location::current()) noexcept safe
{
  unsafe {
    fprintf(stderr,
      "%s:%d:%d\n%s\n%.*s\n",
      loc.file_name(),
      loc.line(),
      loc.column(),
      loc.function_name(),
      msg.size(),
      msg.data());
    fflush(stderr);
    abort();
  }
}

[[noreturn, safety::panic(panic_code::bounds)]]
inline void panic_bounds(
  str msg, source_location loc = source_location::current()) noexcept safe
{
  unsafe {
    fprintf(stderr,
      "%s:%d:%d\n%s\n%.*s\n",
      loc.file_name(),
      loc.line(),
      loc.column(),
      loc.function_name(),
      msg.size(),
      msg.data());
    fflush(stderr);
    abort();
  }
}

////////////////////////////////////////////////////////////////////////////////
// expected.h

template<class T+, class E+>
choice expected {
  [[safety::unwrap]] ok(T),
  err(E);

  T unwrap(self) noexcept safe {
    return match(self) -> T {
      .ok(t)  => rel t;
      .err(e) => panic("{} is err".format(expected~string));
    };
  }
};

////////////////////////////////////////////////////////////////////////////////
// optional.h

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

  template<class E>
  expected<T, E> ok_or(self, E e) noexcept safe {
    return match(self) -> expected<T, E> {
      .some(t) => .ok(rel t);
      .none    => .err(rel e);
    };
  }

  T expect(self, str msg) noexcept safe {
    return match(self) -> T {
      .some(t) => rel t;
      .none    => panic(msg);
    };
  }

  T unwrap(self) noexcept safe {
    return match(rel self) -> T {
      .some(t) => rel t;
      .none    => panic("{} is none".format(optional~string));
    };
  }

  optional<T> take(self^) noexcept safe {
    return replace<optional<T>>(self, .none);
  }

  template<class P>
  optional<T> take_if(self^, P p) safe
  requires FnMut<P, bool, T>
  {
    return match(*self) -> optional<T> {
      .some(^x) => (
        match(p(x)) -> optional<T> {
          true => replace<optional>(self, .none);
          false => .none;
        });
      .none => .none;
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

////////////////////////////////////////////////////////////////////////////////
// iterator.h

interface iterator {
  typename item_type;
  optional<item_type> next(self^) safe;
};

interface make_iter {
  typename iter_type;
  typename iter_mut_type;
  typename into_iter_type;

  iter_type      iter(const self^) safe;
  iter_mut_type  iter(self^) safe;
  into_iter_type iter(self) safe;
};

////////////////////////////////////////////////////////////////////////////////
// slice.h

template<class T>
class slice_iterator/(a)
{
  T* unsafe p_;
  T* end_;
  T^/a __phantom_data;

public:
  slice_iterator([T; dyn]^/a s) noexcept safe
    : p_((*s)~as_pointer), unsafe end_((*s)~as_pointer + (*s)~length)
  {
  }

  optional<T^/a> next(self^) noexcept safe {
    if (self->p_ == self->end_) { return .none; }
    return .some(^*self->p_++);
  }
};

template<class T>
impl slice_iterator<T>: iterator
{
  using item_type = T^;

  optional<item_type> next(self^) safe override {
    return self.next();
  }
};

template<class T>
impl slice_iterator<T>: make_iter {
  using iter_type = slice_iterator<T>;
  using iter_mut_type = slice_iterator<T>;
  using into_iter_type = slice_iterator<T>;

  iter_type      iter(const self^) safe override { return *self; }
  iter_mut_type  iter(self^) safe override { return *self; }
  into_iter_type iter(self) safe override { return self; }
};

////////////////////////////////////////////////////////////////////////////////
// utility.h

interface send {};
interface sync {};

template<class T+>
class manually_drop
{
  T t_;

public:
  explicit
  manually_drop(T t) noexcept safe
    : t_(rel t)
  {
  }

  ~manually_drop() = trivial;

  void destroy(self^) noexcept {
    self->t_^.~T();
  }

  T const^ get(self const^) noexcept safe {
    return ^self->t_;
  }
};

template<class T+>
void forget(T t) noexcept safe
{
  manually_drop<T>(rel t);
}

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

template<class T+>
T replace/(a)(T^/a dst, T src) safe
{
  unsafe {
    T result = __rel_read(addr *dst);
    __rel_write(addr *dst, rel src);
    return result;
  }
}

////////////////////////////////////////////////////////////////////////////////
// atomic.h

template<class T>
class [[unsafe::sync(true)]] atomic
{
  unsafe_cell<std::atomic<T> unsafe>  t_;

public:
  atomic(T t = T()) safe
  : t_(rel t)
  {}

  atomic(atomic const^) = delete;
  operator rel(atomic) = delete;

  T fetch_add(self const^, T op, std::memory_order memory_order = std::memory_order_seq_cst) noexcept safe {
    unsafe { return  self->t_.get()&->fetch_add(op, memory_order) + op; }
  }

  T fetch_sub(self const^, T op, std::memory_order memory_order = std::memory_order_seq_cst) noexcept safe {
    unsafe { return  self->t_.get()&->fetch_sub(op, memory_order); }
  }

  T add_fetch(self const^, T op, std::memory_order memory_order = std::memory_order_seq_cst) noexcept safe {
    unsafe { return  self->t_.get()&->fetch_add(op, memory_order) + op; }
  }

  T sub_fetch(self const^, T op, std::memory_order memory_order = std::memory_order_seq_cst) noexcept safe {
    unsafe { return  self->t_.get()&->fetch_sub(op, memory_order) - op; }
  }

  void store(self const^, T op, std::memory_order memory_order = std::memory_order_seq_cst) noexcept safe {
    unsafe { self->t_.get()&.store(op, memory_order); }
  }

  T load(self const^, std::memory_order memory_order = std::memory_order_seq_cst) noexcept safe {
    unsafe { self->t_.get()&.load(memory_order); }
  }

  T operator++(self const^) noexcept safe {
    return self->add_fetch(1);
  }

  T operator++(self const^, int) noexcept safe {
    return self->fetch_add(1);
  }

  T operator--(self const^) noexcept safe {
    return self->sub_fetch(1);
  }

  T operator--(self const^, int) noexcept safe {
    return self->fetch_sub(1);
  }
};

////////////////////////////////////////////////////////////////////////////////
// arc.h

template<class T+>
class
[[unsafe::send(T~is_send && T~is_sync), unsafe::sync(T~is_send && T~is_sync)]]
arc
{
  struct arc_inner;
  arc_inner* unsafe p_;

  struct arc_inner
  {
    manually_drop<T> data_;
    atomic<std::size_t> strong_;
    atomic<std::size_t> weak_;

    arc_inner(T data) noexcept safe
      : data_(rel data)
      , strong_(1)
      , weak_(1)
    {
    }
  };

public:

  explicit
  arc(T t) safe
    : p_(new(std::nothrow) arc_inner(rel t))
  {
  }

  template<typename T2>
  explicit arc(T2 t) safe :
    arc(T(rel t)) { }

  arc(arc const^ rhs) safe
    : p_(rhs->p_)
  {
    ++p_->strong_;
  }

  [[unsafe::drop_only(T)]]
  ~arc() safe
  {
    std::size_t s = --p_->strong_;
    if (s == 0) {
      unsafe { mut p_->data_.destroy(); }

      std::size_t w = --p_->weak_;
      if (w == 0) {
        delete p_;
      }
    }
  }

  T const^ operator->(self const^) noexcept safe {
    return ^*self->p_->data_.get();
  }
};

////////////////////////////////////////////////////////////////////////////////
// box.h

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
  box(T t) safe
    : p_(new(std::nothrow) T(rel t))
  {
  }

  [[unsafe::drop_only(T)]]
  ~box() safe {
    delete p_;
  }

  static
  box make_default() safe requires(safe(T()))
  {
    unsafe { return box(new T()); }
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

////////////////////////////////////////////////////////////////////////////////
// unique_ptr.h

template<class T+>
using unique_ptr = optional<box<T>>;

////////////////////////////////////////////////////////////////////////////////
// cell.h

template<class T+>
class [[unsafe::sync(false)]] cell
{
  unsafe_cell<T> t_;

  public:

  explicit cell(T t) noexcept safe
    requires(T~is_trivially_copyable && T~is_trivially_destructible)
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

  T replace(self const^, T t) noexcept safe {
    unsafe { auto old = __rel_read(self->t_.get()); }
    unsafe { __rel_write(self->t_.get(), rel t); }
    return old;
  }
};

////////////////////////////////////////////////////////////////////////////////
// initializer_list.h

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

////////////////////////////////////////////////////////////////////////////////
// string.h

template<class CharT>
class basic_string;

using string    = basic_string<char>;
using wstring   = basic_string<wchar_t>;
using u8string  = basic_string<char8_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;

template<class CharT>
class [[unsafe::send, unsafe::sync]]
  __attribute__((preferred_name(string)))
  __attribute__((preferred_name(wstring)))
  __attribute__((preferred_name(u8string)))
  __attribute__((preferred_name(u16string)))
  __attribute__((preferred_name(u32string)))
basic_string
{
  public:

  using value_type = CharT;
  using size_type = std::size_t;

  static_assert(value_type~is_trivially_destructible);

  basic_string() safe
    : p_(nullptr)
    , size_{0}
    , capacity_{0}
  {
  }

  // Make expensive converting ctors explicit.
  // The user should know they're doing a heap allocation.
  explicit
  basic_string(string_constant<value_type> sc) safe
    : basic_string(basic_string_view<value_type>(sc))
  {
  }

  explicit
  basic_string(basic_string_view<value_type> sv) safe
    : basic_string()
  {
    if (sv.empty()) return;

    auto n = sv.size() * sizeof(value_type);
    unsafe { p_ = static_cast<value_type*>(::operator new(n)); }
    unsafe { std::memcpy(p_, sv.data(), n); }
    size_ = sv.size();
    capacity_ = sv.size();
  }

  explicit
  basic_string(const [value_type; dyn]^ init) safe
    : basic_string(basic_string_view<value_type>(init))
  {
  }

  basic_string(basic_string const^ rhs) safe :
    basic_string(rhs.str())
  {
  }

  ~basic_string() safe {
    if (p_)
      unsafe { operator delete(p_); }
  }

  const [value_type; dyn]^ slice(self const^) noexcept safe {
    unsafe { return slice_from_raw_parts(self.data(), self.size()); }
  }

  basic_string_view<value_type> str(self const^) noexcept safe {
    using no_utf_check = typename basic_string_view<value_type>::no_utf_check;
    unsafe { return basic_string_view<value_type>(self.slice(), no_utf_check{}); }
  }

  operator basic_string_view<value_type>(self const^) noexcept safe {
    return self.str();
  }

  value_type const* data(self const^) noexcept safe {
    return self->p_;
  }

  size_type size(self const^) noexcept safe {
    return self->size_;
  }

  size_type capacity(self const^) noexcept safe {
    return self->capacity_;
  }

  void append(self^, basic_string_view<value_type> rhs) safe {
    if (auto len = self.size() + rhs.size(); len > self.capacity()) {
      unsafe { auto p = static_cast<value_type*>(::operator new(len * sizeof(value_type))); }
      unsafe { std::memcpy(p, self.data(), self.size() * sizeof(value_type)); }
      unsafe { ::operator delete(self->p_); }

      self->p_ = p;
      self->capacity_ = len;
    }

    unsafe { std::memcpy(self->p_ + self.size(), rhs.data(), rhs.size() * sizeof(value_type)); };
    self->size_ += rhs.size();
  }

  basic_string operator+(self const^, basic_string_view<value_type> rhs) safe {
    basic_string s = cpy self;
    mut s.append(rhs);
    drp self;
    return rel s;
  }

  private:
  value_type* unsafe p_;
  size_type size_;
  size_type capacity_;
};

namespace literals
{
namespace string_literals
{

inline
auto operator""s2(char const* p, std::size_t len) noexcept safe -> string
{
  unsafe { return string(slice_from_raw_parts(p, len)); }
}

inline
auto operator""s2(char8_t const* p, std::size_t len) noexcept safe -> u8string
{
  unsafe { return u8string(slice_from_raw_parts(p, len)); }
}

inline
auto operator""s2(char16_t const* p, std::size_t len) noexcept safe -> u16string
{
  unsafe { return u16string(slice_from_raw_parts(p, len)); }
}

inline
auto operator""s2(char32_t const* p, std::size_t len) noexcept safe -> u32string
{
  unsafe { return u32string(slice_from_raw_parts(p, len)); }
}

inline
auto operator""s2(wchar_t const* p, std::size_t len) noexcept safe -> wstring
{
  unsafe { return wstring(slice_from_raw_parts(p, len)); }
}

} // namespace string_literals
} // namespace literals

////////////////////////////////////////////////////////////////////////////////
// io.h

inline void println(string_view sv) noexcept safe
{
  unsafe { printf("%.*s\n", sv.size(), sv.data()); }
}

inline void println(string_constant<char> sc) noexcept safe
{
  println(sc.text());
}

inline void println(const std::string^ s) noexcept safe {
  unsafe { printf("%s\n", s->c_str()); }
}

inline void println(signed char x) noexcept safe
{
  unsafe { printf("%hhd\n", x); }
}

inline void println(short x) noexcept safe
{
  unsafe { printf("%hd\n", x); }
}

inline void println(int x) noexcept safe
{
  unsafe { printf("%d\n", x); }
}

inline void println(long int x) noexcept safe
{
  unsafe { printf("%ld\n", x); }
}

inline void println(long long int x) noexcept safe
{
  unsafe { printf("%lld\n", x); }
}

inline void println(unsigned char x) noexcept safe
{
  unsafe { printf("%hhu\n", x); }
}

inline void println(unsigned short x) noexcept safe
{
  unsafe { printf("%hu\n", x); }
}

inline void println(unsigned int x) noexcept safe
{
  unsafe { printf("%u\n", x); }
}

inline void println(long unsigned int x) noexcept safe
{
  unsafe { printf("%lu\n", x); }
}

inline void println(long long unsigned int x) noexcept safe
{
  unsafe { printf("%llu\n", x); }
}

inline void println(float x) noexcept safe {
  unsafe { printf("%f\n", x); }
}

inline void println(double x) noexcept safe {
  unsafe { printf("%f\n", x); }
}

inline void println(long double x) noexcept safe {
  unsafe { printf("%Lf\n", x); }
}

////////////////////////////////////////////////////////////////////////////////
// mutex.h

template<class T+>
class
[[unsafe::send(T~is_send), unsafe::sync(T~is_send)]]
mutex
{
  using mutex_type = unsafe_cell<std::mutex>;

  unsafe_cell<T> data_;
  box<mutex_type> mtx_;


public:
  class lock_guard/(a)
  {
    friend class mutex;

    mutex const^/a m_;

    lock_guard(mutex const^/a m) noexcept safe
      : m_(m)
    {
    }

    public:
    ~lock_guard() safe {
      unsafe { mut m_->mtx_->get()->unlock(); }
    }

    T const^ borrow(self const^) noexcept safe {
      unsafe { return ^*self->m_->data_.get(); }
    }

    T^ borrow(self^) noexcept safe {
      unsafe { return ^*self->m_->data_.get(); }
    }

    T^ operator*(self^) noexcept safe {
      return self.borrow();
    }

    T const^ operator*(self const^) noexcept safe {
      return self.borrow();
    }
  };

  explicit mutex(T data) noexcept safe
    : data_(rel data)
    , unsafe mtx_(box<mutex_type>::make_default())
  {
  }

  mutex(mutex const^) = delete;

  lock_guard lock(self const^) safe {
    unsafe { mut self->mtx_->get()->lock();}
    return lock_guard(self);
  }
};

////////////////////////////////////////////////////////////////////////////////
// rc.h

template<class T+>
class [[unsafe::send(false)]] rc
{
  struct rc_inner;

  rc_inner* unsafe p_;

  struct rc_inner
  {
    manually_drop<T> data_;
    std::size_t strong_;
    std::size_t weak_;

    explicit
    rc_inner(T data) noexcept safe
      : data_(rel data)
      , strong_(1)
      , weak_(1)
    {
    }
  };

  public:

  explicit
  rc(T t) safe
    : p_(new(std::nothrow) rc_inner(rel t))
  {
  }

  rc(rc const^ rhs) safe
    : p_(rhs->p_)
  {
    ++p_->strong_;
  }

  [[unsafe::drop_only(T)]]
  ~rc() safe
  {
    std::size_t s = --p_->strong_;
    if (s == 0) {
      unsafe { mut p_->data_.destroy(); }

      std::size_t w = --p_->weak_;
      if (w == 0) {
        delete p_;
      }
    }
  }

  T const^ operator->(self const^) noexcept safe {
    return *self->p_->data_.get();
  }

  T const^ operator*(self const^) noexcept safe {
    return *self->p_->data_.get();
  }
};


////////////////////////////////////////////////////////////////////////////////
// ref_cell.h

template<class T+>
class ref_cell
{
  unsafe_cell<T> t_;
  cell<int> borrow_count_;

  public:

  class ref/(a)
  {
    friend class ref_cell;

    T* value_;
    cell<int> const^/a borrow_;

    ref(T* value, cell<int> const^/a borrow) noexcept safe
      : value_(value)
      , borrow_(borrow)
    {
      borrow_.set(borrow_.get() + 1);
    }

    public:

    ref(ref const^ rhs) safe
      : ref(rhs.value_, rhs.borrow_)
    {
    }

    ~ref() safe {
      auto b = borrow_.get();
      borrow_.set(b - 1);
    }

    T const^ operator*(self const^) noexcept safe {
      unsafe { return *self->value_; }
    }
  };

  class ref_mut/(a)
  {
    friend class ref_cell;

    T* value_;
    cell<int> const^/a borrow_;
    // T^/a __phantom_data;

    ref_mut(T* value, cell<int> const^/a borrow) noexcept safe
      : value_(value)
      , borrow_(borrow)
    {
      borrow_.set(borrow_.get() - 1);
    }

    public:

    ~ref_mut() safe {
      auto b = borrow_.get();
      borrow_.set(b + 1);
    }

    T const^ operator*(self const^) noexcept safe {
      unsafe { return *self->value_; }
    }

    T^ operator*(self^) noexcept safe {
      unsafe { return ^*self->value_; }
    }
  };

  explicit
  ref_cell(T t) noexcept safe
    : t_(rel t)
    , borrow_count_{0}
  {
  }

  ref_cell(ref_cell const^) = delete;

  optional<ref> try_borrow(self const^) noexcept safe
  {
    auto b = self->borrow_count_.get();
    if (b < 0) return .none;
    return .some(ref{self->t_.get(), self->borrow_count_});
  }

  ref borrow(self const^) noexcept safe {
    auto opt = self.try_borrow();
    return match(rel opt) {
      .some(b) => rel b;
      .none => panic("ref_cell failed to acquire const borrow");
    };
  }

  optional<ref_mut> try_borrow_mut(self const^) noexcept safe
  {
    auto b = self->borrow_count_.get();
    if (b > 0) return .none;
    if (b == -1) return .none;
    return .some(ref_mut{self->t_.get(), self->borrow_count_});
  }

  ref_mut borrow_mut(self const^) noexcept safe {
    auto opt = self.try_borrow_mut();
    return match(rel opt) {
      .some(b) => rel b;
      .none => panic("ref_cell failed to acquire const borrow");
    };
  }

  T^ get_mut(self^) noexcept safe {
    unsafe { return ^*self->t_.get(); }
  }
};

////////////////////////////////////////////////////////////////////////////////

template<class T+>
class
[[unsafe::send(T~is_send), unsafe::sync(T~is_send)]]
shared_mutex
{
  using mutex_type = unsafe_cell<std::shared_mutex>;

  unsafe_cell<T> data_;
  box<mutex_type> mtx_;

public:
  class lock_guard/(a)
  {
    friend class shared_mutex;

    shared_mutex const^/a m_;

    lock_guard(shared_mutex const^/a m) noexcept safe
      : m_(m)
    {
    }

    public:
    ~lock_guard() safe {
      unsafe { m_->mtx_->get()&->unlock(); }
    }

    T const^ borrow(self const^) noexcept safe {
      unsafe { return ^*self->m_->data_.get(); }
    }

    T^ borrow(self^) noexcept safe {
      unsafe { return ^*self->m_->data_.get(); }
    }

    T^ operator*(self^) noexcept safe {
      return self.borrow();
    }

    T const^ operator*(self const^) noexcept safe {
      return self.borrow();
    }
  };

  class shared_lock_guard/(a)
  {
    friend class shared_mutex;

    shared_mutex const^/a m_;

    shared_lock_guard(shared_mutex const^/a m) noexcept safe
      : m_(m)
    {
    }

    public:
    ~shared_lock_guard() safe {
      // TODO: it seems hard to get implementations to trigger tools like helgrind, drd
      // here, even with assertions enabled in libstdc++
      // this was mistakenly a call to `->unlock()` which is incorrect
      // we need some method of verifying we get a failure here if we call the wrong thing
      unsafe { m_->mtx_->get()&->unlock_shared(); }
    }

    T const^ borrow(self const^) noexcept safe {
      unsafe { return ^*self->m_->data_.get(); }
    }

    T const^ operator*(self const^) noexcept safe {
      return self.borrow();
    }
  };

  explicit shared_mutex(T data) noexcept safe
    : data_(rel data)
    , unsafe mtx_(box<mutex_type>::make_default())
  {
  }

  shared_mutex(shared_mutex const^) = delete;

  lock_guard lock(self const^) safe  {
    unsafe { self->mtx_->get()&->lock(); }
    return lock_guard(self);
  }

  shared_lock_guard lock_shared(self const^) safe {
    unsafe { self->mtx_->get()&->lock_shared(); }
    return shared_lock_guard(self);
  }
};

////////////////////////////////////////////////////////////////////////////////
// thread.h

class thread
{
  std::thread unsafe t_;

  template<class F, class ...Args>
  static
  void call((F, (Args...,))* p_tup) safe
  {
    unsafe { box<(F, (Args...,))> p = p_tup; }

    auto tup = p rel.into_inner();
    mut tup.0 rel.(rel tup.1.[:] ...);
  }

public:

  thread() = delete;
  thread(thread const^) safe = delete;

  template<class F+, class ...Args+>
  thread/(where F: static, Args...: static)(F f, Args... args) safe
  requires(
    F~is_send &&
    (Args~is_send && ...) &&
    safe(mut f(rel args...)))
    : unsafe t_()
  {
    static_assert(!__is_lambda(F), "lambdas in std2::thread not yet supported by toolchain");
    using tuple_type = (F, (Args...,));

    box<tuple_type> p{(rel f, (rel args... ,))};
    // TODO: have the thread constructor throw here somehow
    // must catch the case where a clever stdlib dev thinks they can
    // replace p.get() with `p rel.leak()` here, which causes a memory
    // leak upon `std::thread::thread` throwing
    unsafe { t_ = std::thread(&call<F, Args...>, p.get()); }
    forget(rel p);
  }

  ~thread() safe {
    mut t_.detach();
  }

  void join(self) safe {
    mut self.t_.join();

    // TODO: this line will someday be _required_ once ABI issues are worked out
    // and dropping through a relocated function parameter works
    forget(rel self);
  }
};

////////////////////////////////////////////////////////////////////////////////
// vector.h


template<class T+>
class vector;

template<class T+>
class into_iterator
{
  using value_type = T;

  friend class vector<T>;

  value_type* unsafe origin_;
  value_type* unsafe p_;
  value_type* end_;


  public:
  into_iterator(value_type* p, value_type* end)
    : origin_(p)
    , p_(p)
    , end_(end)
  {
  }

  ~into_iterator() safe {
    while (p_ < end_) {
      unsafe { auto t = __rel_read(p_++); }
      (void)t;
    }
    unsafe { ::operator delete(origin_); }
  }

  optional<value_type> next(self^) noexcept safe {
    if (self->p_ < self->end_) {
      unsafe { return .some(__rel_read(self->p_++)); }
    } else {
      return .none;
    }
  }
};

// TODO: make vector conditionally Send/Sync
template<class T+>
class vector
{
public:
  using value_type = T;
  using size_type = std::size_t;

  vector() safe
    : p_(nullptr)
    , capacity_{0}
    , size_{0}
  {
  }

  vector(initializer_list<value_type> unsafe ilist) safe
    : vector()
  {
    self^.reserve(ilist.size());
    unsafe { relocate_array(mut self.data(), ilist.data(), ilist.size()); }
    self.size_ = ilist.size();

    ilist^.advance(ilist.size());
  }

  [[unsafe::drop_only(T)]]
  ~vector() safe {
    // TODO: std::destroy_n() doesn't seem to like `int^` as a value_type
    // eventually we should fix this

    unsafe {
      auto const* end = self.data() + self.size();
      auto* pos = self^.data();

      while (pos < end) {
        std::destroy_at(pos);
        ++pos;
      }

      ::operator delete(p_);
    }
  }

  slice_iterator<const value_type> iter(const self^) noexcept safe {
    return slice_iterator<const value_type>(self.slice());
  }

  slice_iterator<value_type> iter(self^) noexcept safe {
    return slice_iterator<value_type>(self.slice());
  }

  value_type* data(self^) noexcept safe {
    return self->p_;
  }

  const value_type* data(const self^) noexcept safe {
    return self->p_;
  }

  size_type size(const self^) noexcept safe {
    return self->size_;
  }

  size_type capacity(const self^) noexcept safe {
    return self->capacity_;
  }

  bool empty(const self^) noexcept safe {
    return self.size() == 0;
  }

  void push_back(self^, T t) safe {
    if (self.capacity() == self.size()) { self.grow(); }

    __rel_write(self->p_ + self->size_, rel t);
    ++self->size_;
  }

  [value_type; dyn]^ slice(self^) noexcept safe {
    unsafe { return slice_from_raw_parts(self.data(), self.size()); }
  }

  const [value_type; dyn]^ slice(const self^) noexcept safe {
    unsafe { return slice_from_raw_parts(self.data(), self.size()); }
  }

  value_type^ operator[](self^, size_type i) noexcept safe {
    if (i >= self.size()) panic_bounds("vector subscript is out-of-bounds");
    unsafe { return ^self.data()[i]; }
  }
  value_type^ operator[](self^, size_type i, no_runtime_check) noexcept {
    return ^self.data()[i];
  }

  const value_type^ operator[](const self^, size_type i) noexcept safe {
    if (i >= self.size()) panic_bounds("vector subscript is out-of-bounds");
    unsafe { return ^self.data()[i]; }
  }
  const value_type^ operator[](const self^, size_type i, no_runtime_check) noexcept {
    return ^self.data()[i];
  }

  void reserve(self^, size_type n) safe {
    if (n <= self.capacity()) return;

    value_type* p;
    unsafe {
      p = static_cast<value_type*>(::operator new(n * sizeof(value_type)));
      relocate_array(p, self.data(), self.size());
      ::operator delete(self->p_);
    }

    self->p_ = p;
    self->capacity_ = n;
  }

private:

  static
  void relocate_array(value_type* dst, value_type const* src, size_type n) {
    // TODO: we should add a relocation check here
    // this code likely isn't sound for types with non-trivial/non-defaulted
    // relocation operators
    std::memcpy(dst, src, n * sizeof(value_type));
  }

  void grow(self^) safe {
    size_type cap = self.capacity();
    size_type ncap = cap ? 2 * cap : 1;
    self.reserve(ncap);
  }

  value_type* unsafe  p_;
  size_type capacity_;
  size_type size_;
  value_type __phantom_data;
};

template<class T>
impl into_iterator<T>: iterator
{
  using item_type = T;

  optional<item_type> next(self^) safe override {
    return self.next();
  }
};

template<class T>
impl vector<T>: make_iter {
  using iter_type = slice_iterator<T const>;
  using iter_mut_type = slice_iterator<T>;
  using into_iter_type  = into_iterator<T>;

  iter_type iter(self const^) noexcept safe override {
    return slice_iterator<const T>(self.slice());
  }

  iter_mut_type iter(self^) noexcept safe override {
    return slice_iterator<T>(self.slice());
  }

  into_iter_type iter(self) noexcept safe override {
    auto p = self^.data();
    auto len = self.size();
    forget(rel self);
    unsafe { return into_iter_type(p, p + len); }
  }
};

} // namespace std
