# Tinydb's source directory

- This is where all the source code lies.
- This README aims to provide a brief intro to the source code, as well as some
how-to's if you want to contribute. And a todo-list.

## Intro

- There are 4 sub-directories:
  - `general` provides generic information such as versioning and sizes/offsets
  of pages.
  - `dbfile` provides the underlying storage mechanism, and is currently the
  part being focused on. Depends on `general`.
  - `sql` provides the parser and interpreter for the query language. Depends on
  stuff in `general` and (eventually) `dbfile`.
  - `cli` provides the user-facing REPL. Currently only used to test `sql`'s
  parser. Depends on everything preceding.

- In each source file, the header includes should be specified relative to this
directory. For example, to include `page.hxx`, which is in `dbfile/internal`,
one must write `#include "dbfile/internal/page.hxx"`.
- The file `tinydb.cxx` provides the module `tinydb`, if you build with modules.

## Todo

- Add READMEs in all the sub-directories.
  - Focus on `dbfile` first.
