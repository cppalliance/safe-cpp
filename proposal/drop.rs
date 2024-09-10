fn main() {
  let mut v = Vec::<&i32>::new();
  {
    let x = 101;
    v.push(&x);
  }

  // Error: `x` does not live long enough
  drop(v);
}