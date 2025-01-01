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

class PageMeta {
  public:
    template <typename T>
        requires std::derived_from<T, PageTag>
    explicit PageMeta(T&& t_page)
        : m_impl{new PageModel<T>(std::forward<T>(t_page))} {}
    void read_from(std::istream& t_in) { m_impl->read_from(t_in); }
    void write_to(std::ostream& t_out) { m_impl->write_to(t_out); }

  private:
    // NOLINTBEGIN(*special-member*)
    struct PageConcept {
        virtual void read_from(std::istream& t_in) = 0;
        virtual void write_to(std::ostream& t_out) = 0;
        virtual auto clone() -> std::unique_ptr<PageConcept> = 0;
        virtual ~PageConcept() = default;
    };
    // NOLINTEND(*special-member*)
    template <typename T>
        requires std::derived_from<T, PageTag>
    struct PageModel : PageConcept {
        template <typename Tp>
            requires std::convertible_to<Tp, T>
        explicit PageModel(Tp&& t_page) : m_page(std::forward<Tp>(t_page)) {}
        void read_from(std::istream& t_in) override {
            // it seems to not be possible to just name this `read_from` due to
            // conflicting names.
            read_from_impl(m_page, t_in);
        }
        void write_to(std::ostream& t_out) override {
            write_to_impl(m_page, t_out);
        }
        auto clone() -> std::unique_ptr<PageConcept> override {
            return std::make_unique<PageModel>(*this);
        }
        T m_page;
    };

    std::unique_ptr<PageConcept> m_impl;
};

class FreePageMeta : public PageTag {
    friend void read_from_impl(FreePageMeta& t_meta, std::istream& t_in);
    friend void write_to_impl(const FreePageMeta& t_meta, std::ostream& t_out);

  private:
    // not written into the database file.
    uint32_t m_page_num;
    // offset 1: pointer to next page. Set to 0 if this is the last free page.
    //   If set to 0, the next free page should be the page right below this
    //   page in memory order.
    uint32_t m_next_pg;
};

} // namespace tinydb::dbfile

#endif // !TINYDB_DBFILE_PAGE_HXX
