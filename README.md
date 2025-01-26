# Tinydb

## What

- A currently non-functioning table database.

> [!NOTE]
> Just before writing this README, the whole project was in proof-of-concept
> mode. As a concrete implementation seems possible, the project, including this
> README, is undergoing reorganization.
>
> A dev log and a kind-of todo list is being planned.

## Building

### Building: Prerequisites

> [!NOTE]
> A container may be provided in the future to help with all these very new
> versions.
> Also, there isn't a setup for packaging this project yet, so it's not possible
> to run, say, `cmake --build build --target install`.

- A C++ compiler that supports most of C++23. The newest versions of all major
compilers would suffice.
  ~~- So far, testing on g++-14 and clang++-18 indicates that these compilers all
  work.~~
  - clang++-19 or later works flawlessly.
    - clang++-18 cannot find `std::expected`, unless you add `-D CMAKE_CXX_FLAGS:STRING="-stdlib=libc++"`.
    This requires `libc++-dev` and `libc++abi-dev`, both of version 18 or above
    (so, `libc++-18-dev` and `libc++abi-18-dev` on `apt`).
  - MSVC (Visual Studio) works, if your Visual Studio is of version 17 or newer.
  - g++-14 hits a compiler segfault. TLDR; it doesn't work.
    - Maybe version 14.3 has this fixed. [Here's the bug report page of GCC on that
    issue](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=114630).
  - Apple's clang (so, XCode): no.
    - [If you want a peek](https://arewemodulesyet.org/tools/)

> [!NOTE]
> Extended testing on which compiler versions are the absolute minimum hasn't
> been done.

- CMake 3.28+.
- Ninja build (`ninja-build` on `apt`).

- Optional: ccache (speeds up non-module builds, from the second build
onwards).
- Optional: `gcc` or `msvc` users may want to take a look at LLVM for
all the sanitizers used in this project.
  - AddressSanitizer (ASan) is available on all compilers.
    - If available, it is recommended to compile with this sanitizer. Helps
    tremendously with tracking memory bugs (say, use-after-free or memory leak).
  - MemorySanitizer (MSan) or UndefinedBehaviorSanitizer (UBSan) only available
  on LLVM (so, `clang`).
  - If a package manager isn't available at the disposal, check out
  [the release page of LLVM](https://github.com/llvm/llvm-project/releases).

- Optional: Google Test.
  - If you enable unit test without Google Test, the project is going to pull it
  from the internet for you.

### Building: Actually building the project

- Make sure you are in the project's root directory (or, the directory with
this README).
- A CMake preset is provided:

```bash
# not ready for release yet, so this is all I have.
# this Debug version has unit test enabled.
cmake --preset debug
cmake --build --preset debug
# if you want to build AND run the unit test
cmake --workflow --preset debug
```

- If you want more fine-grained configuration, using `ccmake` or `cmake-gui`
is recommended.
  - Or, write your own `CMakeUserPresets.json`.

## Editor setup

- Should work on any editor, or at least those that support CMake.

### Editor setup: No setup

- This project is not dependent on any IDE. Which means you can use just about
any text editor to edit the code.
  - Even Notepad. Sure, no code highlight, no LSP, but you can still compile and
  run the code.

### Editor setup: Neovim

- Use your favorite plugin manager to download `nvim-lspconfig`, `Mason`,
`conform.nvim` and `nvim-lint`.
After that, install `clangd`, `clang-format` and `clang-tidy` (either you
already have this with your LLVM install, or you need to use `Mason`).
- Then, configure `clangd` in `nvim-lspconfig`, `clang-format` in `conform` and
`clang-tidy` in `nvim-lint`.
  - A bit lazy here. Do a Google search for these plugins.
  - Optional: use `Mason` to download `vale` and `markdownlint` if you plan to
  write up the README of this project.

### Editor setup: Code

- Download the C/C++ language extension.
- Download the CMake extension.
  - Now you should have the CMake icon (a triangle) in the left bar.
  - Go there and in the "Configure" tab, select "Debug build without Modules,
  and with unit test".
- Optional: find and download plugins for `markdownlint` and `vale` if you plan
to write up the README of this project.

### Editor setup: Visual studio

> [!NOTE]
> Should work as with any other CMake projects. Not tested yet.
> Do a Google search on "visual studio cmake project."

### Editor setup: Apple XCode

> [!NOTE]
> Should work as with any other CMake projects. Not tested yet.
> Do a Google search on "xcode cmake project."

- You can download `clang` with Homebrew (`brew install llvm`), and set up VS
Code or Neovim using the preceding guides.

### Editor setup: CLion

> [!NOTE]
> Should work as with any other CMake projects. Not tested yet.
> Do a Google search on "clion cmake project."

### Editor setup: Code::Blocks

> [!NOTE]
> Should work as with any other CMake projects. Not tested yet.
> Do a Google search on "codeblocks cmake project."

## Debugger setup

- TLDR:
  - If you use Visual Studio or any IDE, use their built-in debugger.
  - If you use `libstdc++` (so, the default standard library on Linux), use `gdb`.
    - Personal choice. `gdb` even allows you to "step back" the code.
  - If you use `libc++` (default on Mac), use `lldb`.
  - `gdb` and `lldb` are quite difficult to use, but the payoff should be well
  worth it.
    - "Step-back," memory watchpoint, multithreaded debugging, you name it.

## Contribute?

- Fork, create own branch, do work, and create pull request. Nothing crazy.
