// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/optional.h>
#include <std2/utility.h>

namespace std2
{

// `cell` must require Copy, and not Clone, of the underlying type
// this is because it's trivial to create self-referential structs using a construct like:
// https://users.rust-lang.org/t/why-does-cell-require-copy-instead-of-clone/5769/3
//
// use std::cell::UnsafeCell;
//
// // version of Cell with Clone
// struct Cell<T> {
//     data: UnsafeCell<T>
// }
//
// impl<T: Clone> Cell<T> {
//     fn new(x: T) -> Cell<T> {
//         Cell { data: UnsafeCell::new(x) }
//     }
//
//     fn get(&self) -> T {
//         unsafe {(*self.data.get()).clone()}
//     }
//     fn set(&self, x: T) {
//         unsafe {
//             *self.data.get() = x;
//         }
//     }
// }
//
//
// // the "magic" type
// struct BadClone<'a> {
//     data: Vec<i32>,
//     pointer: &'a Cell<Option<BadClone<'a>>>,
// }
//
// impl<'a> Clone for BadClone<'a> {
//     fn clone(&self) -> BadClone<'a> {
//         let data = &self.data;
//         println!("before: {:?}", *data);
//
//         // by assigning None, we run the Drop code, deallocating the Vec
//         self.pointer.set(None);
//
//         // access data after it's freed
//         println!("after: {:?}", *data);
//
//         BadClone { data: self.data.clone(), pointer: self.pointer }
//     }
// }
//
// fn main() {
//     // our stack-local creates stable storage for the Option
//     let cell = Cell::<Option<BadClone<'_>>>::new(None);
//
//     // BadClone is instantiated with the address of our stack-local
//     // we then re-assign it, creating a self-referential structure
//     // BadClone points to the Cell that contains it
//     cell.set(Some(BadClone {
//         data: vec![12345678],
//         pointer: &cell,
//     }));
//
//     // our invalidating action
//     cell.get();
// }

template<class T+>
class [[unsafe::sync(false)]] cell
{
  unsafe_cell<T> t_;

  public:

  explicit
  cell(T t) noexcept safe
    : t_(rel t)
  {
  }

  T get(self const^) safe {
    // rely on implicit copy operator erroring out for types with non-trivial
    // destructors or types that have user-defined copy constructors
    unsafe { return *self->t_.get(); }
  }

  void set(self const^, T t) safe {
    unsafe { *self->t_.get() = rel t; }
  }

  T replace(self const^, T t) safe {
    unsafe { auto old = __rel_read(self->t_.get()); }
    unsafe { __rel_write(self->t_.get(), rel t); }
    return old;
  }
};

} // namespace std2
