/**
 * @file page_base.hxx
 * @brief Defines some basic (but essential) enum/types/variable names for any
 * part of the database that deals with pages.
 */

#ifndef TINYDB_DBFILE_INTERNAL_PAGE_BASE_HXX
#define TINYDB_DBFILE_INTERNAL_PAGE_BASE_HXX

#include "general/modules.hxx"
#ifndef ENABLE_MODULES
#include <cstdint>
#include <type_traits>
#include "general/utils.hxx"
#endif // !ENABLE_MODULES

TINYDB_EXPORT
namespace tinydb::dbfile::internal {

// general numeric type

/**
 * @brief A flag that each page has.
 * Determines how a page is formatted.
 * */
enum class PageType : char {
  Free = 0,
  BTreeLeaf,
  BTreeInternal,
  Heap,
};

enum class PageReadErrCode : uint8_t {
  WrongPageType = 1,
};

// PageType's underlying number type.
using pt_num_t = std::underlying_type_t<PageType>;
using err_num_t = std::underlying_type_t<PageReadErrCode>;

/**
 * @class PageMixin
 * @brief Any page metadata should inherit this, publicly.
 * Adds page number functionality into any page.
 *
 */
class PageMixin {
public:
  PageMixin() = delete;
  /**
   * @return The page number of this page.
   */
  [[nodiscard]] constexpr auto get_pg_num() const noexcept -> uint32_t {
    return m_pg_num;
  }

  // no need for virtual dtor here.

  explicit PageMixin(uint32_t t_pg_num) : m_pg_num{t_pg_num} {}

protected:
  // NOLINTBEGIN(*non-private*)

  // Not written to the database file. Merely a convenient way to keep track of
  // page numbers.
  uint32_t m_pg_num;
  // NOLINTEND(*non-private*)
};

constexpr page_ptr_t NULL_PAGE = 0;

} // namespace tinydb::dbfile::internal

#endif // !TINYDB_DBFILE_INTERNAL_PAGE_BASE_HXX
