#feature on safety
#include <std2.h>
#include <chrono>

using namespace std2;

void entry_point(box<string> data, int thread_id) safe {
  mut data->append("🔥");
  println(*data);
  unsafe { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
}

int main() safe {
  box<string> shared_data(string("Hello world - "));

  vector<thread> threads { };
  for(int i : 10)
    mut threads.push_back(thread(entry_point, cpy shared_data, i));
  
  for(thread t : rel threads)
    t rel.join();
}