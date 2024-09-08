#feature on safety
#include <std2.h>
#include <chrono>

using namespace std2;

// mutex is sync, so arc<mutex<string>> is send.
void entry_point(arc<mutex<string>> data, int thread_id) safe { 
  // Lock the data through the mutex.
  // When lock_guard goes out of scope, the mutex is released.
  auto lock_guard = data->lock();

  // Get a mutable borrow to the string data. When lock_guard goes out of 
  // scope, this becomes a dangling pointer. The borrow checker prevents
  // us from accessing through dangling pointers.
  string^ s = mut lock_guard.borrow();

  // Append a fire and print the new shared state.
  s.append("🔥");

  // Drop the lock before printing the shared state. This makes the borrow
  // `s` a "dangling pointer," in the sense that it depends on an expired
  // lifetime. That will raise a borrowck error on `println(*s)`, which
  // attempts to access shared state outside of the lock.
  // drp lock_guard;

  // Drop the data before printing shared state. This decrements arc's 
  // reference count and could potentially free the string data. It 
  // raises a borrowck error on `println(*s)` because the borrow on `data`
  // from `data->lock()` is kept live by the use of `println(*s)` via the
  // borrow `mut lock_guard.borrow()`.
  // drp data;

  // Print the updated shared state.
  println(*s);

  // Sleep 1s before returning.
  unsafe { std::this_thread::sleep_for(std::chrono::seconds(1)); }
}

int main() safe {
  // arc - Shared ownership.
  // mutex - Shared mutable access.
  arc<mutex<string>> shared_data(string("Hello world - "));

  // Launch 10 threads.
  vector<thread> threads { };
  for(int i : 10)
    mut threads.push_back(thread(entry_point, cpy shared_data, i));
  
  // The consuming into_iterator produced by `rel threads` lets us relocate
  // elements out of the vector for the consuming thread::join call.
  for(thread t : rel threads)
    t rel.join();
}