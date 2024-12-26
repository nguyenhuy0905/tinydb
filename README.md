# Tinydb

> Third attempt at making a database

## Why did it take you so much pain

- Well, didn't define a MVP well enough. And got overwhelmed.
- Yes, that means the current draft is the MVP.

## So what is the MVP

- Single table.
- Each entry is at max about 4000 bytes large.
  - This can be extended later.

- Some basic data types.
  - Unsigned Int (4 bytes).
  - String (say, at most 255 characters).

- Single-indexed.
  - For now, manual `id` insertion is forced. No auto-increment.
  - `id` must be an unsigned int.
  - Simply compare byte-to-byte.
  - Multi-indexing then probably something like a hash table.

## Command-line

- Call `tinydbcli` to open the REPL.

> [!WARNING]
> The following is outdated and is being updated.

### 1. Quickie

- `exit` to quit.
- `help` to print help.
- `version` to print version.
- `open <FILENAME>` to open a database file.
- `close` to close the currently opened database file.
- `create <TABLENAME> <DIR>` to create file `<DIR>/<TABLENAME>.tinydb`.
- `format name type size name type size...` to create a new table in the opened file.
Invalidates all other data stored in the table.
  - `type`s accepted for now: `uint`, `strg`.
  - It's a bit inconvenient: an int is 4 bytes always but one still needs to
  enter the length.
- `query all` to print out all entries.
- `query tblname` to print out current table name. Prints an empty line if no
file is currently opened.
- `query colname` to print out column names.
- `query coldata` to print out column names and sizes.
- `query id=ID` to get the row with the specified ID.
  - As of now only ID query is thought of.
  - May also go `id<ID`, `id>ID`, `id<=ID`, `id>=ID`, `id?`. The last
  option prints a random row.
- `insert (id ...)` to add a row into the table. ID must be the first specified
element. Data must be sequentially ordered just like how the table is declared.
- `behead id=ID` to remove the row with the specified ID.

## Format of a database file

### 1. File header

#### 1.1. Table definition

- Table name: it's just the filename. 0 byte.
- Some validation string: "SMALLPP\0\0\0." 10 bytes.
- Content type: in the format `name<TAB>type<TAB>size<TAB>`.
  - So long as the table doesn't exceed 4000 bytes, it's good.
  - Type: flagged. 1 = uint, 2 = string.
  - Content ends with `\0`, so, for example:

  ```txt
  id    1   4   col1  1   4   col2    2   255   col3    2   69\0
  ```

#### 1.2. More metadata

- Number of entries: 4 bytes.
- Root pointer: pointer to B+ Tree root page. 4 bytes.
- Free list pointer: pointer to head of free list. 4 bytes.

### 2. B+ tree

- Each node can be of 2 types: an internal page (index page) or a leaf page
(data page). Your usual B+ tree. All node has fixed size equivalent to a
predefined page size (4096).

#### 2.1. Index page format

- Flag: 1 byte: tell that it's an index page.
- n: 2 bytes. The index page has n pointers and n - 1 rowid.
- Stores rowid and pointer to a child page.
  - rowid: 4 bytes
  - pointer: 4 bytes
  - stored like so:

  ```txt
  -- n pointers, n - 1 rowid
  ptr0 | rowid0 | ptr1 | rowid1 | ... | ptrn
  ```

  - ptr0 points to child page whose entries/children pages all contain
  rowid less than rowid0
  - ptr1 then greater than rowid0 and less than rowid1.
  - and so on.

#### 2.2. Data page format

- Stores data. Of course.
- This page is a leaf page so it has no more pointers.
- Flag: 1 byte: tell that it's a data page.
- n: 2 bytes. There are n entries in this page.
- Data.

### 3. Free list

- Singly linked list of free pages. The tail points to 0, indicating that the
page right after it, and everything from there, is free.
- After a B+ tree node is removed, it's added as the head of free list.
- Content of free list:
- Flag: 1 byte: Tell that it's a free page.
- n: 2 bytes. Always 0. Just to be more in sync with other types of pages.
- Pointer: 4 byte: The next free page.

## In-memory format

- Used when processing data in the database file.

### 1. Metadata

- tblname: string.
- table.
- number of entries.
- root and freelist pointers.

### 2. Page

- Flag (or, page type).
- Number of pointers/entries.
- Raw bytes. Processed depending on page type.

## Future upgrades

- New types:
  - Unsigned long: 8 bytes.
