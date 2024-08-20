#feature on safety

#include <std2/vector.h>
#include <std2/string_view.h>

int main() safe {
  std2::vector<std2::string_view> strs = {};
  {
    const char buf[] = {'h', 'e', 'l', 'l', 'o' };
    std2::string_view sv = buf;
    strs^.push_back(sv);
  }

  strs.size();
}
