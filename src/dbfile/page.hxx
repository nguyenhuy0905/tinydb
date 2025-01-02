#ifndef TINYDB_DBFILE_PAGE_HXX
#define TINYDB_DBFILE_PAGE_HXX

#include <cassert>
#include <concepts>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <type_traits>

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

/**
 * @class PageTag
 * @brief Any page should inherit this tag.
 *
 */
class PageTag {};

/**
 * @class PageMeta
 * @brief Type-erased page metadata.
 *
 */
class PageMeta {
  public:
    template <typename T>
        requires std::is_base_of_v<PageTag, T>
    explicit PageMeta(T&& t_page)
        : m_impl{new PageModel<T>(std::forward<T>(t_page))} {}
    /**
     * @brief Reads the page metadata from the given stream.
     *   The content read is written to the page held under this `PageMeta`.
     *
     * @param t_in The given stream.
     *   The stream must inherit from `std::istream`.
     */
    void read_from(std::istream& t_in) { m_impl->read_from(t_in); }
    /**
     * @brief Writes the content of the page this `PageMeta` holds into the
     * given stream.
     *
     * @param t_out The given stream.
     *   The stream must inherit from `std::ostream`
     */
    void write_to(std::ostream& t_out) { m_impl->write_to(t_out); }

    // special members

    PageMeta(PageMeta&&) = default;
    auto operator=(PageMeta&&) -> PageMeta& = default;
    PageMeta(const PageMeta& t_meta) : m_impl{t_meta.m_impl->clone()} {}
    auto operator=(const PageMeta& t_meta) -> PageMeta& {
        m_impl = t_meta.m_impl->clone();
        return *this;
    }
    ~PageMeta() = default;

  private:
    // NOLINTBEGIN(*special-member*)

    /**
     * @class PageConcept
     * @brief Standardized name.
     * @details A pure virtual struct.
     *
     */
    struct PageConcept {
        virtual void read_from(std::istream& t_in) = 0;
        virtual void write_to(std::ostream& t_out) const = 0;
        virtual auto clone() -> std::unique_ptr<PageConcept> = 0;
        virtual ~PageConcept() = default;
    };
    // NOLINTEND(*special-member*)

    // NOLINTBEGIN(*special-member*)

    /**
     * @class PageModel
     * @tparam T Must inherit from PageTag.
     * @brief The inner type that holds the actual page metadata.
     *
     */
    template <typename T>
        requires std::is_base_of_v<PageTag, T>
    struct PageModel : PageConcept {
        template <typename Tp>
            requires std::convertible_to<Tp, T>
        explicit PageModel(Tp&& t_page) : m_page(std::forward<Tp>(t_page)) {}
        void read_from(std::istream& t_in) override {
            // it seems to not be possible to just name this `read_from` due to
            // conflicting names.
            read_from_impl(m_page, t_in);
        }
        void write_to(std::ostream& t_out) const override {
            write_to_impl(m_page, t_out);
        }
        auto clone() -> std::unique_ptr<PageConcept> override {
            return std::make_unique<PageModel>(*this);
        }
        ~PageModel() override = default;
        T m_page;
    };
    // NOLINTEND(*special-member*)

    std::unique_ptr<PageConcept> m_impl;
};

/**
 * @class FreePageMeta
 * @brief Contains metadata about free page
 *
 */
class FreePageMeta : PageTag {
  public:
    explicit FreePageMeta(uint32_t t_page_num)
        : m_page_num{t_page_num}, m_next_pg{0} {}
    FreePageMeta(uint32_t t_page_num, uint32_t t_next_pg)
        : m_page_num{t_page_num}, m_next_pg{t_next_pg} {}

    void set_next_pg(uint32_t t_next_pg) { m_next_pg = t_next_pg; }
    [[nodiscard]] auto next_pg() const -> uint32_t { return m_next_pg; }

  private:
    // not written into the database file.
    uint32_t m_page_num;
    // offset 1: pointer to next page. Set to 0 if this is the last free page.
    //   If set to 0, the next free page should be the page right below this
    //   page in memory order.
    uint32_t m_next_pg;

    friend void read_from_impl(FreePageMeta& t_meta, std::istream& t_in);
    friend void write_to_impl(const FreePageMeta& t_meta, std::ostream& t_out);
};

class BTreeLeafMeta : PageTag {
  public:
    explicit BTreeLeafMeta(uint32_t t_page_num)
        : m_page_num{t_page_num}, m_n_rows{0}, m_first_free{DEFAULT_FREE_OFF} {}
    BTreeLeafMeta(uint32_t t_page_num, uint16_t t_n_rows, uint16_t t_first_free)
        : m_page_num{t_page_num}, m_n_rows{t_n_rows},
          m_first_free{t_first_free} {}

  private:
    // not written into the database file.
    uint32_t m_page_num;
    // offset 1: number of rows stored inside this leaf.
    uint16_t m_n_rows;
    // offset 3: the first free offset.
    //   default to 5, since 4 is the last byte of the metadata chunk.
    uint16_t m_first_free;
    static constexpr uint16_t DEFAULT_FREE_OFF = 5;

    friend void read_from_impl(BTreeLeafMeta& t_meta, std::istream& t_in);
    friend void write_to_impl(const BTreeLeafMeta& t_meta, std::ostream& t_out);
};

} // namespace tinydb::dbfile

#endif // !TINYDB_DBFILE_PAGE_HXX
