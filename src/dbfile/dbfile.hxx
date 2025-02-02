#ifndef TINYDB_DBFILE_DBFILE_HXX
#define TINYDB_DBFILE_DBFILE_HXX

#include "general/modules.hxx"
#ifndef ENABLE_MODULES
#include "dbfile/coltype.hxx"
#include "dbfile/internal/freelist.hxx"
#include "dbfile/internal/page_base.hxx"
#include "dbfile/internal/tbl.hxx"
#include "version.hxx"
#include <bit>
#include <cstdint>
#include <iostream>
#include <memory>
#endif // !ENABLE_MODULES

EXPORT
namespace tinydb::dbfile {

/**
 * @class DbFile
 * @brief The database file itself.
 *
 */
class DbFile {
public:
  /**
   * @brief Read the stream passed in and constructs a database file.
   * Note, with the current implementation, there is no checking whether the
   * file is valid.
   * Since the bottleneck is the I/O anyways, extensive checking could
   * technically be implemented.
   *
   * @param t_io The stream.
   */
  static auto construct_from(std::unique_ptr<std::iostream> t_io) -> DbFile;

  /**
   * @brief Nuke the stream to prepare writing a new database into it.
   *
   * @param t_io The stream.
   */
  static auto new_empty(std::unique_ptr<std::iostream> t_io) -> DbFile;

  /**
   * @brief Writes the database metadata into the stream it was assigned to.
   * Equivalent to nuking the database if you write_init into an already
   * formatted file.
   */
  void write_init();

  /**
   * @brief Add a column into the table.
   * Equivalent to nuking the database if you add a column into an
   * already-written database file.
   *
   * @param t_name The column name. Must be special. Embrace difference.
   * @param t_coltype The column type.
   */
  void add_column(std::string&& t_name, column::ColType t_coltype);

private:
  template <class T1, class T2>
    requires std::is_same_v<T1, internal::TableMeta> &&
                 std::is_same_v<T2, internal::FreeList>
  DbFile(T1&& t_tbl, T2&& t_fl, std::unique_ptr<std::iostream> t_io)
      : m_tbl{std::forward<T1>(t_tbl)}, m_rw{std::move(t_io)},
        m_freelist{std::forward<T2>(t_fl)} {}
  internal::TableMeta m_tbl;
  std::unique_ptr<std::iostream> m_rw;
  internal::FreeList m_freelist;
  // for now there's no need for heap yet.
};

} // namespace tinydb::dbfile
#endif // !TINYDB_DBFILE_DBFILE_HXX
