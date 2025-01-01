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
 * @details
 */
class PageMeta {
  public:
    template <typename T>
        requires std::derived_from<T, PageTag>
    PageMeta(T&& t_page)
        : m_pimpl(new PageMethod<T>(std::forward<T>(t_page))){};
    void write_to(std::ostream& t_out) { m_pimpl->write_to(t_out); }
    void read_from(std::istream& t_in) { m_pimpl->read_from(t_in); }

  private:
    // NOLINTBEGIN(*special-member*)
    class IPage {
      public:
        IPage() = delete;
        virtual void write_to(std::ostream& t_out) = 0;
        virtual void read_from(std::istream& t_in) = 0;
        virtual auto clone() -> std::unique_ptr<IPage> = 0;
        virtual ~IPage() = default;
    };
    // NOLINTEND(*special-member*)
    template <typename T>
        requires std::derived_from<T, PageTag>
    class PageMethod : IPage {
      public:
        template <typename Tp>
            requires std::convertible_to<Tp, T>
        explicit PageMethod(Tp&& t_page) : m_page(std::forward<Tp>(t_page)) {}
        void write_to(std::ostream& t_out) override { write_to(m_page, t_out); }
        void read_from(std::istream& t_in) override { read_from(m_page, t_in); }
        auto clone() -> std::unique_ptr<IPage> override {
            return std::make_unique<PageMethod<T>>(*this);
        }
        T m_page;
    };

    std::unique_ptr<IPage> m_pimpl;
};

// Note, friend functions are not member functions.

/**
 * @class FreePageMeta
 * @brief Contains metadata of a free page.
 *
 */
class FreePageMeta : public PageTag {
  public:
    friend void write_to(const FreePageMeta& t_page, std::ostream& t_out);
    friend void read_from(const FreePageMeta& t_page, std::istream& t_in);

  private:
    uint32_t m_pagenum;
    uint32_t m_next_pg;
};

/**
 * @class FreePageMeta
 * @brief Contains metadata of a leaf page.
 *
 */
class BTreeLeafMeta : public PageTag {
  public:
    friend void write_to(const BTreeLeafMeta& t_page, std::ostream& t_out);
    friend void read_from(const BTreeLeafMeta& t_page, std::istream& t_in);

  private:
    uint32_t m_pagenum;
    uint16_t m_nrows;
    uint16_t m_last_offset;
};

} // namespace tinydb::dbfile

#endif // !TINYDB_DBFILE_PAGE_HXX
