#ifndef TINYDB_INTERNAL_DBFILE_PAGE_HPP
#define TINYDB_INTERNAL_DBFILE_PAGE_HPP

#ifndef TINYDB_MODULE
#include <cstddef>
#endif // !TINYDB_MODULE

#ifndef TINYDB_PAGESIZE
constexpr size_t PAGESIZE = 4096;
#else
constexpr size_t PAGESIZE = TINYDB_PAGESIZE;
#endif // !TINYDB_PAGESIZE

#ifndef TINYDB_MODULE
namespace tinydb::internal::dbfile {
#else
export namespace tinydb::internal::dbfile {
#endif

}

#endif // !TINYDB_INTERNAL_DBFILE_PAGE_HPP
