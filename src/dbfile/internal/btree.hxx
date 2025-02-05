/**
 * @file btree.hxx
 * @brief Declares internal interface for BTree.
 *
 * NOT IMPLEMENTED YET.
 */

#include "general/modules.hxx"
#ifndef ENABLE_MODULES
#include "dbfile/internal/tbl.hxx"
#include <cstddef>
#include <iosfwd>
#include <span>
#include <vector>
#endif // !ENABLE_MODULES

#ifndef TINYDB_DBFILE_INTERNAL_BTREE_HXX
#define TINYDB_DBFILE_INTERNAL_BTREE_HXX

namespace tinydb::dbfile::internal {

/**
 * @brief The B+ Tree.
 * The TableMeta being passed in should be the same one every time.
 */
TINYDB_EXPORT
class BTree {
  // TODO: sit at a board and design this thing.
public:
  BTree();
  void add_row(const TableMeta&, std::iostream&);

private:
  auto get_row_bytes(const TableMeta&, std::span<std::byte>, std::string_view,
                     std::iostream&) -> std::vector<std::byte>;
};

} // namespace tinydb::dbfile::internal

#endif // !TINYDB_DBFILE_INTERNAL_BTREE_HXX
