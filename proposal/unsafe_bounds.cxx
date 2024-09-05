#feature on safety
#include <std2.h>

void subscript_array([int; 10] array, size_t i, size_t j) safe {
  array[i; unsafe] += array[j; unsafe];
}

void subscript_slice([int; dyn]^ slice, size_t i, size_t j) safe {
  slice[i; unsafe] += slice[j; unsafe];
}

void subscript_vector(std2::vector<int> vec, size_t i, size_t j) safe {
  mut vec[i; unsafe] += vec[j; unsafe];
}