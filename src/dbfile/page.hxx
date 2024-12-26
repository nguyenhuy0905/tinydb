#ifndef TINYDB_DBFILE_PAGE_HXX
#define TINYDB_DBFILE_PAGE_HXX

#include <cstdint>
#include <filesystem>
#include <type_traits>
namespace tinydb::dbfile {

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

// PageType's underlying number type.
using pt_num = std::underlying_type_t<PageType>;

// NOLINTBEGIN(*-special-member-functions*)
/**
 * @class PageMeta
 * @brief Contains metadata about a page.
 * This is an abstract class.
 *
 * Probably gonna make this an interface.
 */
class PageMeta {
    // NOLINTEND(*-special-member-functions*)
  public:
    // TODO: implement page metadata.
    //
    // I will probably use inheritance for this one. There are a few `PageMeta`
    // types, as listed in the `PageType` enum.

    /**
     * @brief Your usual virtual destructor.
     */
    virtual ~PageMeta();
    /**
     * @brief Write the content of this page into the specified file.
     *
     * @param path The path to the specified file.
     */
    virtual void write_to(const std::filesystem::path& t_path) = 0;

    // both write_to and read_from are dealing with I/O, so the cost of virual
    // is, virtually, nothing.

    /**
     * @brief Reads the page data from the specified file.
     *
     * @param t_path The path to the specified file.
     */
    virtual void read_from(const std::filesystem::path& t_path) = 0;
    /**
     * @return The page number of this page.
     */
    auto get_page_num() -> uint16_t { return m_pagenum; }

  protected:
    /**
     * @brief This is declared as protected so that the end-user cannot create
     * this shit.
     *
     * @param t_pagenum
     */
    explicit PageMeta(uint16_t t_pagenum) : m_pagenum(t_pagenum) {}
    // NOLINTBEGIN(*-private-member-variables-*)
    uint16_t m_pagenum;
    // NOLINTEND(*-private-member-variables-*)
};

} // namespace tinydb::dbfile

#endif // !TINYDB_DBFILE_PAGE_HXX
