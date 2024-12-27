#ifndef TINYDB_DBFILE_PAGE_HXX
#define TINYDB_DBFILE_PAGE_HXX

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <type_traits>
#include <iosfwd>
namespace tinydb::dbfile {

constexpr uint16_t PAGESIZ = 4096;

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
    virtual ~PageMeta() = default;
    /**
     * @brief Write the content of this page into the specified file.
     *
     * @param path The path to the specified file.
     */
    virtual void write_to(std::ostream& t_out) = 0;

    // both write_to and read_from are dealing with I/O, so the cost of virual
    // is, virtually, nothing.

    /**
     * @brief Reads the page data from the specified file.
     *
     * @param t_path The path to the specified file.
     */
    virtual void read_from(std::istream& t_in) = 0;
    /**
     * @return The page number of this page.
     */
    auto get_page_num() -> uint32_t { return m_pagenum; }

  protected:
    /**
     * @brief This is declared as protected so that the end-user cannot create
     * this shit.
     *
     * @param t_pagenum
     */
    explicit PageMeta(uint32_t t_pagenum) : m_pagenum(t_pagenum) {
        // the 0th page is the metadata header.
        assert(t_pagenum > 0);
    }
    // NOLINTBEGIN(*-private-member-variables-*)
    uint32_t m_pagenum;
    // NOLINTEND(*-private-member-variables-*)
};

// NOLINTBEGIN(*-special-member-functions*)

/**
 * @class FreePageMeta
 * @brief Contains metadata about free page.
 *
 */
class FreePageMeta : public PageMeta {
// NOLINTEND(*-special-member-functions*)
  public:
    FreePageMeta(uint32_t t_pagenum, uint32_t t_n_page, uint32_t t_p_next_page)
        : PageMeta(t_pagenum), m_n_pages(t_n_page),
          m_p_next_page(t_p_next_page) {}
    /**
     * @brief Initializes a placeholder page meta object. One should call
     * read_from after this.
     *
     * @param t_pagenum
     * @return
     */
    static inline auto placeholder(uint32_t t_pagenum) -> FreePageMeta {
        return FreePageMeta{t_pagenum};
    }
    void write_to(std::ostream& t_out) override;
    void read_from(std::istream& t_in) override;
    ~FreePageMeta() override;

  private:
    explicit FreePageMeta(uint32_t t_pagenum)
        : PageMeta(t_pagenum), m_n_pages(0), m_p_next_page(0) {}
    // Format:
    // offset 0: 1-byte set to 0 (indicating free page)
    // offset 1: 4-byte, the size of this freepage in number-of-pages.
    //   If number is 0, this page and anything below it is free.
    // offset 5: 4-byte pointer to the next free page.
    // the rest: raw bytes of nothingness.
    uint32_t m_n_pages;
    uint32_t m_p_next_page;
};

} // namespace tinydb::dbfile

#endif // !TINYDB_DBFILE_PAGE_HXX
