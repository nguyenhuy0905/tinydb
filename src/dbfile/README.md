# Tinydb's underlying storage mechanism

- In this directory, there are 3 parts:
  - The source files, intended to be the user-facing API. Depends on `internal`.
  - `internal`, NOT intended to be user-facing API. Depends on the `coltype.hxx`
  source file.
  - `test`, unit test. Depends on everything preceding.

## Todo

~~- Reorganize some parts of `page.hxx`. Some classes (specifically, `HeapMeta`)
shouldn't need to know about `allocate` or `deallocate`; that's the `DbFile`'s
job, really.~~
  ~~- Also, this part used to be somewhat user-facing. Now that it's not, change
  all the `friend` functions into non-friend. `friend`s do you no good.
    - Can be done with a couple of getters/setters.~~ Done.

~~- Move decl of `Ptr` from inside the `dbfile/internal/heap.hxx` to a new header
file inside the `dbfile/internal` directory. `Ptr` is needed in some more places.~~
The entire repo now uses modules. So, very few headers remained.

- Finish the functionalities of the heap.
  - ~~`malloc`~~, ~~`free`~~.
  - ~~`free` should also coalesce free fragments that are next to each
  other, if it can create a combined fragment whose size is a power of 2.~~
~- Change TextType to simply hold a pointer (so, it's 6 bytes in size).~ Kind of
done, I guess?
- If the user wants to set a string-type row as the key, auto-add an 8-byte column
(or more like, whatever size\_t equals to) that holds the hashcode.
  - Maybe a quad-probe solution? By just adding some numbers to the hashcode.
  Double-hash is super costly for long strings.
  - For shorter strings, I'm probably better off comparing letter-by-letter.
- Write up B+ Tree.
  - Restriction: a B+ Tree page must fit at least 4 data rows.
    - A page is usually 4096 bytes, a B+ Tree page header is 5 bytes, leaving 4091
    bytes as data. So, each row must be at most 1022 bytes large.
    - This can be relaxed to "must fit at least 4 keys", but not until I have a working
    B+ Tree.
    - This does mean I may need a pointer-to-row type. Which also means this row
    shouldn't have a key.
    - Reach: later on, I can abstract this away. But, not until I have a working
    B+ Tree. By abstracting away, I mean the program automatically detects that
    you need more memory than 1022 bytes, and allocates.
  - `add_row`, `delete_row`, `get_row`.
    - Pre and post conditions will be defined later.
- Maybe add a fixed-length string type, for anything fewer-than-64-characters.
  - I haven't done benchmarking yet, but I think this is fast enough to compare
  character-by-character. 64 bytes fit on your usual register anyways.
  - Maybe I shouldn't hardcode the value 64 bytes either. Let's say, `sizeof(size_t)`.
- Also, maybe not hard-code 4096 as page size either. I can use something like
`getconf PAGESIZE` on Unix systems. On Windows though.
- Make a B+ Tree hash table for secondary keys functionality.
- Maybe use a "write handler" instead of eagerly writing into the database. Cuz,
we all know database writes are slow, so how about we delay doing that. Or, a
"commit" system. But, this is optimization that should be done much much later.
