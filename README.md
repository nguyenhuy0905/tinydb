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

- A C++ compiler that supports most of C++23. The newest versions of all major
compilers would suffice.
  - So far, testing on g++-14 and clang++-18 indicates that these compilers all
  work.
  - MSVC (Visual Studio) should work if it's the latest Visual Studio.
  - Apple's clang (so, XCode), pretty sure not.

> [!NOTE]
> Extended testing on which compiler versions are the absolute minimum hasn't
> been done.

- CMake 3.30+.

> [!WARNING]
> `apt` on stable Debian or any of its derivatives (say, Ubuntu or Mint) don't
> supply this version of CMake. Debian sid (the bleeding-edge Debian) seems to
> though.
> Also, one techinically only needs 3.30+ if compiling using modules.
> A less-strict version requirement is going to be added.

- Optional: Ninja Build on Linux/Mac (faster build speed than Makefiles).
- Optional: ccache (speeds up non-module builds, from the second build
onwards).

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

## Contribute?

- Fork, create own branch, do work, and create pull request. Nothing crazy.
