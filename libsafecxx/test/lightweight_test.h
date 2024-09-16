// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2.h>

using namespace std2::literals::string_literals;

struct failed_assert {
  std2::string msg_;
  std2::source_location loc_;

  failed_assert(std2::string msg, std2::source_location loc) noexcept safe
    : msg_(rel msg)
    , loc_(rel loc)
  {
  }

  // TODO: should this be unsafe?
  failed_assert(failed_assert const& rhs) safe
    : msg_(cpy rhs->msg_)
    , loc_(cpy rhs->loc_)
  {
  }
};

failed_assert
make_failed_assert(
  std2::string msg,
  std2::source_location loc) safe
{
  return failed_assert(rel msg, rel loc);
}

template<class T, class U>
void require_eq_impl(
  T const^ t, U const^ u,
  std2::source_location loc = std2::source_location::current()) safe
{
  if (!(*t == *u))
    throw make_failed_assert(std2::string("unequal arguments"), loc);
}

void require_impl(
  bool b, std2::source_location loc = std2::source_location::current()) safe
{
  if (!b)
    throw make_failed_assert(std2::string("expected boolean expression was false"), loc);
}

template<class F>
void require_throws_impl(F f, std2::source_location loc = std2::source_location::current()) safe
{
  bool threw = false;
  try {
    f();
  } catch(...) {
    threw = true;
  }

  if (!threw)
    throw make_failed_assert(std2::string("function didn't throw as expected"), loc);
}

#define REQUIRE_EQ(x, y) require_eq_impl((x), (y))
#define REQUIRE(x) require_impl((x))
#define REQUIRE_THROWS(x) require_throws_impl((x))

struct test_runner {
  using fp_type = void(*)() safe;

  std2::vector<fp_type> test_fns_;
  std2::vector<failed_assert> fails_;

  test_runner() safe = default;

  void add_test(self^, fp_type fp) safe {
    mut self->test_fns_.push_back(fp);
  }

  int run(self) safe {
    for (auto fn : self.test_fns_) {
      try {
        fn();
      } catch(failed_assert const& fa) {
        mut self.fails_.push_back(cpy fa);
      }
    }

    if (self.fails_.size() > 0) {
      for (failed_assert const^ fail : self.fails_) {
        std2::println("tests failed!");
        unsafe {
          printf(
            "%.*s at %s(), line %d:%d in %s\n",
            fail->msg_.str().size(), fail->msg_.str().data(),
            fail->loc_.function_name(), fail->loc_.line(), fail->loc_.column(),
            fail->loc_.file_name());
        }
      }
      unsafe { printf("%llu of %llu tests failed\n", self.fails_.size(), self.test_fns_.size()); };
      unsafe { fflush(stdout); }
      drp self;
      return 1;
    }

    unsafe { printf("%d of %d tests passed\n", self.test_fns_.size(), self.test_fns_.size()); }
    drp self;
    return 0;
  }
};

#define TEST_MAIN(...)                              \
int main() safe                                     \
{                                                   \
  using fp_type = void(*)() safe;                   \
  std2::vector<fp_type> test_fns = { __VA_ARGS__ }; \
  test_runner runner{};                             \
  for (auto fn : test_fns) {                        \
    mut runner.add_test(fn);                        \
  }                                                 \
  return runner rel.run();                          \
}
