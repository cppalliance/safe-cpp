# Building the Library and Tests

```cmake
set -e
git clone https://github.com/cppalliance/safe-cpp.git
cd safe-cpp
cmake -S. -B_build -DCMAKE_CXX_COMPILER=circle -DCMAKE_CXX_STANDARD=20 -DSAFE_CXX_BUILD_TESTING=ON
cmake --build _build -j20
ctest --test-dir _build --test-action memcheck
```
