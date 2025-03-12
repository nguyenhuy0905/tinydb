/**
 * @file dbfile.hxx
 * @brief Declares the user-facing interface for handling a database file.
 *
 * NOTE: The interface of reading and constructing a database file will probably
 * be delegated to a new class. This interface is supposed to be used only for
 * modifying rows of the database.
 *
 * WARNING: most of the interface is not yet defined. DO NOT USE.
 *
 * It is possible to assign any arbitrary stream as the database file, so long
 * as that stream derives from `std::iostream`. For example, one can mock-test a
 * database using an `std::stringstream`.
 */

#ifndef TINYDB_DBFILE_DBFILE_HXX
#define TINYDB_DBFILE_DBFILE_HXX

#include "tinydb_export.h"
#ifndef ENABLE_MODULES
#include "dbfile/coltype.hxx"
#include "dbfile/internal/freelist.hxx"
#include "dbfile/internal/tbl.hxx"
#include <iostream>
#include <memory>
#endif // !ENABLE_MODULES

#ifdef ENABLE_MODULES
export namespace tinydb::dbfile {
#else
namespace tinydb::dbfile {
#endif // ENABLE_MODULES

/**
 * @class DbFile
 * @brief The database file itself.
 *
 * Once a database file is created, one can NOT modify its columns.
 */
class TINYDB_EXPORT DbFile {
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
   * @brief Writes the database metadata into the stream it was assigned to.
   * Equivalent to nuking the database if you write_init into an already
   * formatted file.
   */
  void write_init();

  /**
   * @return The name of the current key.
   *
   * If no key was set, it returns a nullopt.
   */
  [[nodiscard]] auto get_key() const -> std::optional<std::string_view>;

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
