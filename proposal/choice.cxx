#feature on safety

// An enum-like choice that has alternatives but no payloads.
// Unlike enums, it is not permitted to cast between C1 and its underlying
// integral type.
choice C1 {
  A, B, C,
};

// A choice type with payloads for each alternative.
choice C2 {
  i8(int8_t),
  i16(int16_t),
  i32(int32_t),
  i64(int64_t),
};

// A choice type with mixed payload and no-payload alternatives.
choice C3 {
  // default allowed once per choice, on a no-payload alternative.
  default none,
  i(int),
  f(float),
};

int main() safe {
  // choice-initializer uses scope resolution to name the alternative.
  C1 x1 = C1::B(); 

  // abbreviated-choice-name infers the choice type from the lhs.
  // If there's no payload type, using () is optional.
  C1 x2 = .B();
  C1 x3 = .B;

  // Create choice objects with initializers.
  C2 y1 = C2::i32(55);
  C2 y2 = .i32(55);

  // If there's a defaulted alternative, the choice type has a default
  // initializer that makes an instance with that value.
  C3 z1 { };
  C3 z2 = C3();
  C3 z3 = C3::none();
  C3 z4 = .none;
}