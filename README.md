# Tinydb

## What

- A currently non-functioning table database.

> [!NOTE]
> Just before writing this README, the whole project was in proof-of-concept
> mode. As a concrete implementation seems possible, the project, including this
> README, is undergoing reorganization.
>
> Planning a dev log of some sort.

## Building

### Building: Prerequisites

> [!NOTE]
> There may be a container to help build the project, since the requirements are
> quite new versions.
> Also, there isn't a setup for packaging this project yet, so it's not possible
> to run, say, `cmake --build build --target install`.

- A C++ compiler that supports most of C++23. The newest versions of all major
compilers would suffice.
  - clang++-19 or later works flawlessly.
    - clang++-18 can't find `std::expected`, unless you add `-D CMAKE_CXX_FLAGS:STRING="-stdlib=libc++"`.
    This requires `libc++-dev` and `libc++abi-dev`, both of version 18 or higher.
    (so, `libc++-18-dev` and `libc++abi-18-dev` on `apt`).
  - Visual Studio works, if it's version 17 or newer.
  - g++-14 hits a compiler segfault with a module build. For a non-module build,
  there is no issue.
    - Maybe version 14.3 has this fixed. [Here's the bug report page of GCC on that
    issue](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=114630).
  - Apple's clang: no.
    - [If you want a peek](https://arewemodulesyet.org/tools/)

> [!NOTE]
> There is no testing on which compiler versions are the absolute minimum.

- CMake 3.28+.
- Ninja build.

- Optional: ccache (speeds up non-module builds, from the second build
onwards).
- Optional: `gcc` or `msvc` users may want to take a look at `llvm` for
all the sanitizers used in this project.
  - AddressSanitizer is available on all compilers.
    - If available, recommended to compile with this sanitizer. Helps
    tremendously with tracking memory bugs.
  - MemorySanitizer or UndefinedBehaviorSanitizer only available
  on Unix platforms.
  - If a package manager isn't available at the disposal, check out
  [the release page of llvm](https://github.com/llvm/llvm-project/releases).

- Optional: Google Test.
  - If you enable unit test without Google Test, the project is going to pull it
  from the internet for you.

### Building: Actually building the project

- Make sure you are in the project's root directory (or, the directory with
this README).
- There is a premade CMake preset.

```bash
# not ready for release yet, so this is all I have.
# this Debug version has unit test enabled.
cmake --preset debug
cmake --build --preset debug
# if you want to build AND run the unit test
cmake --workflow --preset debug
```

- If you want more fine-grained configuration; `ccmake` or `cmake-gui` recommended.
  - Or, write your own `CMakeUserPresets.json`.

## Contribute?

- Fork, create own branch, do work, and create pull request. Nothing crazy.
