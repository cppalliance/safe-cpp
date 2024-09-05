#pragma once

namespace std2 {

// N must be deduced by the implementation.
template<size_t N>
struct subarray_size { };

// Add to operator[] functions for faster calls in `unchecked` contexts.
struct no_runtime_check { };

}