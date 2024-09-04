# Building the Library and Tests

```bash
git clone https://github.com/cppalliance/safe-cpp.git
cd safe-cpp
cmake -Slibsafecxx -B_build -DCMAKE_CXX_COMPILER=circle -DCMAKE_CXX_STANDARD=20 -DSAFE_CXX_BUILD_TESTING=ON
cmake --build _build -j20
ctest --test-dir _build --test-action memcheck
```

# Installing the Library

```bash
git clone https://github.com/cppalliance/safe-cpp.git
cd safe-cpp
cmake -Slibsafecxx -B_build -DCMAKE_CXX_COMPILER=circle -DCMAKE_CXX_STANDARD=20 -DCMAKE_INSTALL_PREFIX=_install
cmake --build _build --target install
```

## Using the Installed Library

```cmake
# use -DCMAKE_PREFIX_PATH=safe-cpp/_install, if not installed into system paths
find_package(SafeCXX REQUIRED COMPONENTS safe_cxx)

add_executable(hello hello.cxx)
target_link_libraries(hello PRIVATE SafeCXX::safe_cxx)
```
