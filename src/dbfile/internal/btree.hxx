/**
 * @file btree.hxx
 * @brief Declares internal interface for BTree.
 *
 * NOT IMPLEMENTED YET.
 */

#include "general/modules.hxx"
#ifndef ENABLE_MODULES
#include "dbfile/internal/page_meta.hxx"
#include "dbfile/internal/tbl.hxx"
#include <cstddef>
#include <iosfwd>
#include <span>
#include <unordered_map>
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
  // I dunno if I can do any better than `span` for now.
  // `span` is the value of the key.

  /**
   * @brief Adds a row with the specified data into the tree.
   * @return Whether the row was added.
   * The map passed in must contain ALL data for a row.
   * (Probably need a row builder or something for this).
   */
  void add_row(std::unordered_map<std::string_view, std::span<std::byte>>,
               std::iostream&);
  /**
   * @brief Removes the row with the specified key, or nothing if there's no
   * such row.
   * @return The removed row if removed successful, nothing otherwise.
   */
  auto remove_row(std::span<std::byte>, std::iostream&);
  /**
   * @brief `remove_row` without the remove part.
   * @return The row if it is in the tree, nothing otherwise.
   */
  auto get_row(std::span<std::byte>, std::iostream&);

private:
  auto split_leaf_node(BTreeLeafMeta&);
  auto split_internal_node(BTreeInternalMeta&);

  TableMeta m_tbl;
};

} // namespace tinydb::dbfile::internal

#endif // !TINYDB_DBFILE_INTERNAL_BTREE_HXX
