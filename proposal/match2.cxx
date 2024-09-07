#feature on safety
#include <std2.h>

using namespace std2;

choice Primitive {
  i8(int8_t),
  u8(uint8_t),
  i16(int16_t),
  u16(uint16_t),
  i32(int32_t),
  u32(uint32_t),
  i64(int64_t),
  u64(uint64_t),
  s(string);

  bool is_signed(const self^) noexcept safe {
    // concise form. Equivalent to Rust's matches! macro.
    return match(*self; .i8 | .i16 | .i32 | .i64);
  }

  bool is_unsigned(const self^) noexcept safe {
    return match(*self; .u8 | .u16 | .u32 | .u64);
  }

  bool is_string(const self^) noexcept safe {
    return match(*self; .s);
  }
};

int main() safe {
  println(Primitive::i16(5i16).is_signed());
  println(Primitive::u32(100ui32).is_unsigned());
  println(Primitive::s("Hello safety").is_string());
}
