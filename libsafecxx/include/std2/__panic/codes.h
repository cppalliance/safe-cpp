#pragma once
#feature on safety

#include <string>

namespace std2
{

// These must be coordinated with the compiler.
enum class panic_code {
  generic,
  bounds,
  divide_by_zero,
  lifetime,
};

} // namespace std2
