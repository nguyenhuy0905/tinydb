/**
 * @file btree.hxx
 * @brief Declares internal interface for BTree.
 *
 * NOT IMPLEMENTED YET.
 */

#include "tinydb_export.h"
#ifndef ENABLE_MODULES
#include "dbfile/internal/page_meta.hxx"
#include "dbfile/internal/tbl.hxx"
#include <cstddef>
#include <iosfwd>
#include <span>
#endif // !ENABLE_MODULES

#ifndef TINYDB_DBFILE_INTERNAL_BTREE_HXX
#define TINYDB_DBFILE_INTERNAL_BTREE_HXX

#ifdef ENABLE_MODULES
export namespace tinydb::dbfile::internal {
#else
namespace tinydb::dbfile::internal {
#endif // ENABLE_MODULES

/**
 * @brief The B+ Tree.
 * The TableMeta being passed in should be the same one every time.
 */
class TINYDB_EXPORT BTree {
  // TODO: sit at a board and design this thing.
public:
  BTree();
  // I dunno if I can do any better than `span` for now.
  // `span` is the value of the key.

  void add_row(std::span<std::byte>, std::iostream&);
  auto remove_row(std::span<std::byte>, std::iostream&);
  auto get_row(std::span<std::byte>, std::iostream&);

private:
  auto split_leaf_node(BTreeLeafMeta&);
  auto split_internal_node(BTreeInternalMeta&);

  TableMeta m_tbl;
};

} // namespace tinydb::dbfile::internal

#endif // !TINYDB_DBFILE_INTERNAL_BTREE_HXX
