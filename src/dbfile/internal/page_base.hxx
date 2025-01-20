#ifndef TINYDB_DBFILE_INTERNAL_PAGE_BASE_HXX
#define TINYDB_DBFILE_INTERNAL_PAGE_BASE_HXX

#include "general/modules.hxx"
#include <cstdint>
#ifndef ENABLE_MODULE
#include <type_traits>
#endif // !ENABLE_MODULE

TINYDB_EXPORT
namespace tinydb::dbfile::internal {

/**
 * @brief A flag that each page has.
 * Determines how a page is formatted.
 * */
enum class PageType : uint8_t {
    Free = 0,
    BTreeLeaf,
    BTreeInternal,
    Heap,
};

enum class PageReadError : uint8_t {
    WrongPageType = 1,
};

// PageType's underlying number type.
using pt_num = std::underlying_type_t<PageType>;
using err_num = std::underlying_type_t<PageReadError>;

/**
 * @class PageMixin
 * @brief Any page should inherit this, publicly.
 * @details Adds page number functionality into any page.
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
    uint32_t m_pg_num;
    // NOLINTEND(*non-private*)
};

template <typename Pg>
concept IsPageMeta =
    requires { std::is_base_of_v<PageMixin, std::remove_cvref_t<Pg>>; };

} // namespace tinydb::dbfile::internal

#endif // !TINYDB_DBFILE_INTERNAL_PAGE_BASE_HXX
