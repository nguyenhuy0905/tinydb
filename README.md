# Cmake template (2.0)

> A rewrite of this CMake template, trimming away some "bloat"

## What's this

- A somewhat minimalist CMake project configuration.

## What's included

- Integration with multiple tools:
  - Formatters & linters:
    - clang-format
    - clang-tidy
    - cmake-format
  - Unit test (GTest), fuzz test (libFuzzer), benchmark (Google benchmark)
  - Sanitizers (ASan, UBSan, MSan, TSan)
  - Download dependencies with conan
  - ccache
  - lld
- Choice to use either C++20 modules or traditional `#include`s.
- Some convenient commands in [the Makefile](./Makefile).
- Basic installation and CPack configuration.

## How to use

- If you want to use conan, first, set up conan [by Reading The Friendly Manual](https://docs.conan.io/2/installation.html):
- Then install packages,

```bash
# build type: either Debug, Release, MinSizeRel or RelWithDebInfo. Case-sensitive.
# default to Debug.
conan install <project source dir> --build=missing -s build_type=<your build type>
# or, you can use the convenient Makefile
# say, I also want to install CMake and Ninja
make conan-install \
    CONAN_OPTIONS="install_cmake=True install_ninja=True" \
    BUILD_TYPE="<your build type>"
```

- Build dir after this step is `<project root>/<your build type>`.
- Then generate the CMake

```bash
# --preset conan-release if building release
# if you want to use the default compiler, remove the
# -DCMAKE_C(XX)_COMPILER and -G option.
# maybe -DCMAKE_EXPORT_COMPILE_COMMANDS also, if you don't need it.
cmake -B <your build dir> --preset conan-debug \
    -DCMAKE_CXX_COMPILER=<the compiler> \
    -DCMAKE_C_COMPILER=<the compiler> \
    -G <generator> \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

- Further configure with ccmake:

```bash
ccmake -B <your build dir>
```

- Lastly, create a symlink if you do need the compile\_commands.json:

```bash
# assuming you're at the project root
ln -s <your build dir>/compile_commands.json compile_commands.json
```

## What to change

- At the very least, the project name, and the export macro.
  - A [shell script](./sed-all.sh) is provided for this task.

  ```bash
  # assuming you're at the project root
  sh ./sed-all.sh
  ```

- If this is a top-level project, and you want these features:
  - [The install configuration](./cmake/InstallConfig.cmake)
  - [The packing configuration](./cmake/PackConfig.cmake)

- If you use other dependencies:
  - [The conanfile](./conanfile.py).
  - Search for the package you want on [ConanCenter](https://conan.io/center).
  - Also, don't forget to read the friendly manual of the package you're consuming.
  - Find package before you use:

  ```cmake
  # eg, using raylib
  find_package(raylib REQUIRED)
  # using the stuff
  add_library(my_game)
  target_sources(my_game PRIVATE my_game_source.cxx)
  target_link_libraries(my_game PRIVATE raylib)
  ```

## Note

- Limited testing was done for MSVC (Visual Studio). While it is expected that
this works out of the box for Visual Studio, in reality, it may not be so simple.

- Option between C++20 modules and traditional headers uses some dirty tricks:
  - Make sure to use either Ninja or Visual Studio as your generator.
  - First, when module is enabled, `ENABLE_MODULE` is defined.
  - Then, there's a header named "module\_cfg.h" in the source directory root.
    - The essentials of this header is the macro `<PROJECT_NAME>_EXPORT`. This
    macro is redefined as `export` when module is enabled, and nothing otherwise.
  - Since preprocessor macro cannot contain other preprocessor macros, one has
  to manually `#ifdef ENABLE_MODULE`. Something like,

  ```cxx
  #ifdef ENABLE_MODULE
  module;
  #else
  #include "lib.hxx"
  #endif

  #include <print>

  #ifdef ENABLE_MODULE
  export module lib;
  #endif
  ```

  - And in the file that imports the module:

  ```cxx
  #ifdef ENABLE_MODULE
  import lib;
  #else
  #include "lib.hxx"
  #endif
  ```

  - For most compilers, standard library header is still experimental, and hence
  not configured with this CMake template.

## Future plans

- Support for ast\_grep
  - Such a cool tool. But have to learn about it first.
