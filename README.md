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

### 1. Quickie

- `exit` to quit.
- `help` to print help.
- `version` to print version.
- `open <FILENAME>` to open (or create) a database file.
- `close` to close the currently opened database file.
- `create <TBLNAME>{name1,type1;name2,type2;...}` to create a table in the
opened file.
  - Some types (vchr) needs a length specifier. varchar(255) for
  example, creates a character array length 255.
  - `type`s accepted for now: `uint`, `vchr`.
  - Maximum length is 255.
- `query all` to print out all entries.
- `query tblname` to print out current table name. Prints an empty line if no
file is currently opened.
- `query colname` to print out column names.
- `query coldata` to print out column names and sizes.
- `query id=ID` to get the row with the specified ID.
  - As of now only ID query is thought of.
  - May also go `id<ID`, `id>ID`, `id<=ID`, `id>=ID`, `id?`. The last
  option prints a random row.
- `insert {name1,name2,...}{val1,val2,...}` to add a row into the table.

> [!WARNING]
> The following is outdated and is being updated.

## Format of a database file

### 1. File header

#### 1.1. Version (0 byte offset)

- 2 bytes major, 2 bytes minor, 2 bytes patch.

#### 1.2. Freelist pointer (6 bytes offset)

- 4-byte pointer to the first free page.

#### 1.3. Table definition

- Variable length, ends at the first closing curly brace.
- Format: `tblname{col1name,col1id,col1size;...;colnname,colnid,colnsize;}`
  - In the future, coltype may be used instead of colsize, when there's
  a better-defined type system.

### 2. Page types

#### 2.1. General

- All pages start with one byte indicating the page type.

#### 2.2. Free page (flag 0)

- offset 1: 4-byte, number of pages under the current free page, including
itself. This number being 0 means this free page is the last page in the
entire database file.
- offset 5: 4-byte, the pointer to the next page. 0 means this free page is
the last free page. This also means that, the number-of-pages is set to 0.

#### 2.3. Btree leaf

- To be added.

#### 2.4. Btree internal

- To be added.

#### 2.5. Heap page

- To be added.

## Future upgrades

- New types:
  - Unsigned long: 8 bytes.
  - Variable-length data types: 4 bytes (pointer).
    - Say, text or any big (more-than-255-byte) stuff.
