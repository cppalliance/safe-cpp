#feature on safety
#include <std2.h>

choice Primitive {
  i8(int8_t),
  i16(int16_t),
  i32(int32_t),
  i64(int64_t),
  i64_2(int64_t),
  i64_3(int64_t),
  pair(int, int),
  s(std2::string)
};

int test(Primitive obj) noexcept safe {
  return match(obj) {
    // Match 1, 2, 4, or 8 for either i32 or i64.
    .i32(1|2|4|8) | .i64(1|2|4|8)   => 1;

    // Match less than 0. ..a is half-open interval.
    .i32(..0)                       => 2;

    // Match 100 to 200 inclusive. ..= is closed interval.
    .i32(100..=200)                 => 3;

    // variant-style access. Match all alternatives with 
    // a `int64_t` type. In this case, i64, i64_2 or i64_3
    // matches the pattern.
    {int64_t}(500 | 1000..2000)     => 4;

    // Match a 2-tuple/aggregate. Bind declarations x and y to 
    // the tuple elements. The match-guard passes when x > y.
    .pair([x, y]) if (x > y)        => 5;

    // Match everything else.
    // Comment the wildcard for an exhaustiveness error.
    _                               => 6;
  };
}

int main() safe {
  int r1 = test(.i64(4));
  std2::println(r1);

  int r2 = test(.i32(150));
  std2::println(r2);

  int r3 = test(.i64_2(1999));
  std2::println(r3);

  int r4 = test(.pair((100, 99)));
  std2::println(r4);

  int r5 = test(.i32(15));
  std2::println(r5);
}
