// A test for https://github.com/cppalliance/safe-cpp/issues/72
#feature on safety
#include <std2.h>
using namespace std2;

int main() safe {
  int i = 0;
  int j = 1;
  vector<const int^> v { ^const i, ^const j }; 

  const int^ bi = v[0];

  // Mutation of i while the loan in v is live should make this
  // ill-formed.
  i = 1;

  // Keep the loan in scope.
  int x = *bi;
}