# Tinydb's underlying storage mechanism

- In this directory, there are 3 parts:
  - The source files, intended to be the user-facing API. Depends on `internal`.
  - `internal`, NOT intended to be user-facing API. Depends on the `coltype.hxx`
  source file.
  - `test`, unit test. Depends on everything preceding.

## Todo

- Reorganize some parts of `page.hxx`. Some classes (specifically, `HeapMeta`)
shouldn't need to know about `allocate` or `deallocate`; that's the `DbFile`'s
job, really.
  - Also, this part used to be somewhat user-facing. Now that it's not, change
  all the `friend` functions into non-friend. `friend`s do you no good.
    - Can be done with a couple of getters/setters.

- Finish the functionalities of the heap.
  - `allocate`, `deallocate`.
  - `deallocate` should also coalesce free fragments that are next to each
  other.

- Move decl of `Ptr` from inside the `dbfile/internal/heap.hxx` to a new header
file inside the `dbfile/internal` directory. `Ptr` is needed in some more places.
- Write the heap (generic allocator, aka, "bad" allocator).
- Change TextType to simply hold a pointer.
- Write up B+ Tree.
  - What to do is specified later.
