fn subscript_array(mut array: [i32; 10], i: usize, j: usize) {
  unsafe { *array.get_unchecked_mut(i) += *array.get_unchecked(j); }
}

fn subcript_slice(slice: &mut [i32], i: usize, j: usize) {
  unsafe { *slice.get_unchecked_mut(i) += *slice.get_unchecked(j); }
}

fn subscript_vector(mut vec: Vec<i32>, i: usize, j: usize) {
  unsafe { *vec.get_unchecked_mut(i) += *vec.get_unchecked(j); }
}