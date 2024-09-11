#feature on safety

#include <std2.h>

using namespace std2;

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
    match(x_.get()) {
      .some(r) => println(*r->p_);
      .none => void();
    };
  }
};

int main() safe
{
  {
    S s1(box<int>(1234));
    S s2(box<int>(4321));
    s1.x_.set(.some(^const s2));
    s2.x_.set(.some(^const s1));
  }
}

// outputs:
// ❯ valgrind ./cell_box_uaf
// ==352616== Memcheck, a memory error detector
// ==352616== Copyright (C) 2002-2022, and GNU GPL'd, by Julian Seward et al.
// ==352616== Using Valgrind-3.22.0 and LibVEX; rerun with -h for copyright info
// ==352616== Command: ./cell_box_uaf
// ==352616==
// 1234
// ==352616== Invalid read of size 4
// ==352616==    at 0x4013E2: S::~S() (in /home/exbigboss/cpp/circle-root/safe-cpp/cell_box_uaf)
// ==352616==    by 0x401294: main (in /home/exbigboss/cpp/circle-root/safe-cpp/cell_box_uaf)
// ==352616==  Address 0x4e2c0d0 is 0 bytes inside a block of size 4 free'd
// ==352616==    at 0x484A164: operator delete(void*) (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
// ==352616==    by 0x4015B5: std2::box<int>::~box() (in /home/exbigboss/cpp/circle-root/safe-cpp/cell_box_uaf)
// ==352616==    by 0x4013FE: S::~S() (in /home/exbigboss/cpp/circle-root/safe-cpp/cell_box_uaf)
// ==352616==    by 0x40128A: main (in /home/exbigboss/cpp/circle-root/safe-cpp/cell_box_uaf)
// ==352616==  Block was alloc'd at
// ==352616==    at 0x48479E8: operator new(unsigned long, std::nothrow_t const&) (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
// ==352616==    by 0x4012D6: std2::box<int>::box(int) (in /home/exbigboss/cpp/circle-root/safe-cpp/cell_box_uaf)
// ==352616==    by 0x401223: main (in /home/exbigboss/cpp/circle-root/safe-cpp/cell_box_uaf)
// ==352616==
// 4321
// ==352616==
// ==352616== HEAP SUMMARY:
// ==352616==     in use at exit: 0 bytes in 0 blocks
// ==352616==   total heap usage: 4 allocs, 4 frees, 74,760 bytes allocated
// ==352616==
// ==352616== All heap blocks were freed -- no leaks are possible
