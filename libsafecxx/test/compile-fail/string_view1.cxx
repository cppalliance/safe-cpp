#feature on safety

#include <std2/string_view.hxx>

int main()
{
  std2::string_view sv;
  {
    [char; 4] buf = {};
    sv = buf;
  }
  (void) sv.size();
}
