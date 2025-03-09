/**
 * @file freelist.hxx
 * @brief Declares an internal interface for freelist.
 *
 */

#ifndef TINYDB_DBFILE_INTERNAL_FREELIST_HXX
#define TINYDB_DBFILE_INTERNAL_FREELIST_HXX

#include "tinydb_export.h"
#ifndef ENABLE_MODULES
#include "dbfile/internal/page_base.hxx"
#include "dbfile/internal/page_serialize.hxx"
#include <iosfwd>
#endif // !ENABLE_MODULES

#ifdef ENABLE_MODULES
export namespace tinydb::dbfile::internal {
#else
namespace tinydb::dbfile::internal {
#endif // ENABLE_MODULES

/**
 * @class FreeList
 * @brief Contains data about the freelist of a database file.
 *
 * The streams passed into any function of this class is expected to
 * be the same, or at the very least, points to the same file/string.
 * This is guaranteed if the FreeList is only used in context of a database
 * file. The end-user shouldn't tinker with this interface anyways.
 *
 * The free list manages free pages. See page meta.
 * It can keep track, "allocate" and "deallocate" free pages.
 * - "Allocate" means repurposing a free page into a different page type.
 *   - If it runs out of free pages, it simply creates a new free page.
 * - "Deallocate" means repurposing a page of different type back into a free
 * page.
 *
 */
class TINYDB_EXPORT FreeList {
public:
  // This should be written into the header of the database file.
  // This is constant-sized so I expect this to be written before the table
  // definition.

  /**
   * @brief Reads the content of the freelist from the specified database
   * file.
   *
   * @param t_in The specified database file.
   */
  static auto construct_from(std::istream& t_in) -> FreeList;

  /**
   * @brief Default-initializes a FreeListMeta, and writes its content to the
   * database file.
   *
   * @param t_in The specified database file.
   */
  static auto default_init(uint32_t t_first_free_pg, std::ostream& t_out)
      -> FreeList;

  void do_write_to(std::ostream& t_out);

  /**
   * @brief Allocates a page of the specified page type, and formats the page
   * accordingly.
   * @tparam T The type of page to be allocated.
   * @tparam Args types of the arguments. Should be deduced by the compiler.
   * @param t_io The stream to read from and write to.
   * @param t_args The parameters, except the page number, required to
   * construct the page.
   * @return The constructed page.
   * @details Use this when you don't need the type-erasure from PageMeta.
   *   However, the I/O is way more expensive than some dynamic casting
   *   anyways.
   *
   *   If you want a PageMeta instead, simply wrap the return value of this
   * function inside a PageMeta constructor.
   *
   */
  template <typename T, typename... Args>
    requires std::is_base_of_v<PageMixin, T>
  auto allocate_page(std::iostream& t_io, Args... t_args) -> T {
    auto page = T{next_free_page(t_io), t_args...};
    write_to(page, t_io);
    return page;
  }

  /**
   * @brief Deallocates the page in the database pointed to by the PageMeta.
   * @details There is no safety check done. One can deallocate the header
   * page, which essentially destroys the entire database.
   *
   * @param t_meta The specified page. Only PageMixin is needed, since I
   * really only care about the page number.
   */
  void deallocate_page(std::iostream& t_io, PageMixin&& t_meta);

private:
  FreeList(uint32_t t_first_free_pg) : m_first_free_pg{t_first_free_pg} {}
  // first free page.
  uint32_t m_first_free_pg;

  /**
   * @brief Returns the current `m_first_free_pg`, reads the stream passed in,
   * and updates `m_first_free_pg` to be the next free page.
   * @return The current `m_first_free_pg`.
   */
  [[nodiscard]] auto next_free_page(std::iostream& t_io) -> page_ptr_t;
};

} // namespace tinydb::dbfile::internal

#endif // !TINYDB_DBFILE_INTERNAL_FREELIST_HXX
