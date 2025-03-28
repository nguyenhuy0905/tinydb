# Building

Requires:

- CMake 3.28 or up.
  - Maybe 3.24 is enough, if you don't build modules.
- GCC 14+ or Clang 19+, or Visual Studio 2022.
  - Still, with GCC 14, module builds are fuvked.
- Conan
  - Otherwise, libfmt, and gtest if you build testing.

## Build

```sh
# debug build
conan install . -s build_type=Debug -b missing
cmake --preset=dev-conan-unix
cmake --build build/Debug
# testing, if you do configure that
ctest --test-dir build/Debug
# release build
conan install . -s build_type=RelWithDebInfo -b missing
cmake --preset=rel-conan-unix
cmake --build build/RelWithDebInfo
```
