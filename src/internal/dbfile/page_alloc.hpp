#ifndef TINYDB_INTERNAL_DBFILE_PAGE_ALLOC_HPP
#define TINYDB_INTERNAL_DBFILE_PAGE_ALLOC_HPP

#ifndef TINYDB_MODULE
#include "internal/dbfile/defs.hpp"
#endif // !TINYDB_MODULE

#ifndef TINYDB_MODULE
namespace tinydb::internal::dbfile {
#else
export namespace tinydb::internal::dbfile {
#endif // !TINYDB_MODULE

enum class NodeColor : bool {
  Red,
  Black,
};

struct PageAllocNode {
  static data_ptr ROOT_NODE;
  data_ptr left_node;
  data_ptr right_node;
  page_ptr_t first_page;
  page_num_t size;
  NodeColor color;
};

constexpr page_offset_t PAGE_ALLOC_SIZE =
    (DATA_PTR_SIZE * 2) + PAGE_PTR_SIZE + PAGE_NUM_SIZE + 1;
}
#endif // !TINYDB_INTERNAL_DBFILE_PAGE_ALLOC_HPP
