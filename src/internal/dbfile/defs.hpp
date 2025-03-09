/**
 * @file defs.hpp
 * @brief Simple definitions for use in dbfile.
 */

#ifndef TINYDB_INTERNAL_DBFILE_DEFS_HPP
#define TINYDB_INTERNAL_DBFILE_DEFS_HPP

#include <cstdint>

namespace tinydb::internal::dbfile {

// NOLINTBEGIN(*identifier-naming*)
using page_ptr_t = uint32_t;
using page_num_t = uint32_t;
using page_offset_t = uint16_t;
struct data_ptr {
  page_ptr_t page;
  page_offset_t offset;
};
// NOLINTEND(*identifier-naming*)

/// @{
/// Sizes that will be written on disk, which doesn't really care about memory alignment.
constexpr page_offset_t PAGE_PTR_SIZE = sizeof(page_ptr_t);
constexpr page_offset_t PAGE_NUM_SIZE = sizeof(page_num_t);
constexpr page_offset_t PAGE_OFFSET_SIZE = sizeof(page_offset_t);
constexpr page_offset_t DATA_PTR_SIZE = PAGE_PTR_SIZE + PAGE_OFFSET_SIZE;
/// @}

}

#endif // !TINYDB_INTERNAL_DBFILE_DEFS_HPP
