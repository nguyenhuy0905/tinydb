# Contributing

## Code of conduct

Read [CODE\_OF\_CONDUCT.md](CODE_OF_CONDUCT.md).

## Style

We use clang-format and clang-tidy for stylistic choices.

Snake-case function names, pascal-case class names.

## Language features

Try to limit template use.

We use C++23. Prefer sum types like `optional` or `expected` over exceptions.
Maybe file I/O is a good place to use exceptions instead.

For now, we care more about correctness than performance. We will tune
performance (which, may include throwing a bunch of code outta the window) once
we find a satisfying design and decide to settle in.
That's not to say one can just, ditch performance concerns. Just keep it in the
back of your mind :).

## Compilation

I heard that GCC 15 will have improved module support, and `import std`. But
until then, we will have to support this dual-build style.
Also, tooling (mostly, clangd and clang-tidy) tends to be confused by modules.

The project has 2 dev presets, one build without modules and one with modules.
`dev-unix-conan` and `dev-unix-conan-module`.
For now, only clang really works well on Unix systems for these module builds.
So, a conan profile for that is also provided.

## License

For now, there's no reason you'd ever use this other than to play around; for
that, it's on Unlicense.
Once I find this project to be nice-enough, I will switch to a GPL/AGPL.
