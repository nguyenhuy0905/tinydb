#ifndef TINYDB_DBFILE_PAGE_HXX
#define TINYDB_DBFILE_PAGE_HXX

#include "modules.hxx"
#ifndef ENABLE_MODULE
#include <cassert>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <type_traits>
#include <vector>
#endif // !ENABLE_MODULE

TINYDB_EXPORT
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
    [[nodiscard]] auto get_pg_num() -> uint32_t { return m_pg_num; }

    // no need for virtual dtor here.

    explicit PageMixin(uint32_t t_pg_num) : m_pg_num{t_pg_num} {}

  protected:
    // NOLINTBEGIN(*non-private*)
    uint32_t m_pg_num;
    // NOLINTEND(*non-private*)
};

/**
 * @class PageMeta
 * @brief Type-erased page metadata.
 *
 */
class PageMeta {
  public:
    template <typename T>
        requires std::is_base_of_v<PageMixin, T>
    explicit PageMeta(T&& t_page)
        : m_impl{new PageModel<T>(std::forward<T>(t_page))} {}
    /**
     * @brief Reads the data from the specified input stream, starting at the
     * specified page number (usually 4096 * page number bytes).
     *
     * @tparam T The page type to be constructed.
     * @param t_in The specified input stream.
     * @param t_pg_num The page number.
     * @return The PageMeta object containing the page constructed.
     *   Or, an exception if the stream is configured to throw, and a read error
     *   occurs.
     */
    template <typename T>
        requires std::is_base_of_v<PageMixin, T>
    static auto construct_from(std::istream& t_in,
                               uint32_t t_pg_num) -> PageMeta {
        PageMeta ret{T{t_pg_num}};
        ret.read_from(t_in);
        return ret;
    }
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

    /**
     * @return The page number of this `PageMeta`.
     */
    auto get_pg_num() -> uint32_t { return m_impl->page_num(); }

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
        virtual auto page_num() -> uint32_t = 0;
        virtual ~PageConcept() = default;
    };
    // NOLINTEND(*special-member*)

    // NOLINTBEGIN(*special-member*)

    /**
     * @class PageModel
     * @tparam T Must inherit from PageMixin.
     * @brief The inner type that holds the actual page metadata.
     *
     */
    template <typename T>
        requires std::is_base_of_v<PageMixin, T>
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
        auto page_num() -> uint32_t override { return m_page.get_pg_num(); }
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
class FreePageMeta : public PageMixin {
  public:
    explicit FreePageMeta(uint32_t t_page_num)
        : PageMixin{t_page_num}, m_next_pg{0} {}
    FreePageMeta(uint32_t t_page_num, uint32_t t_next_pg)
        : PageMixin{t_page_num}, m_next_pg{t_next_pg} {}

    [[nodiscard]] auto get_next_pg() const -> uint32_t { return m_next_pg; }

  private:
    // offset 1: pointer to next page. Set to 0 if this is the last free page.
    //   If set to 0, the next free page should be the page right below this
    //   page in memory order.
    uint32_t m_next_pg;

    friend void read_from_impl(FreePageMeta& t_meta, std::istream& t_in);
    friend void write_to_impl(const FreePageMeta& t_meta, std::ostream& t_out);
};

class BTreeLeafMeta : public PageMixin {
  public:
    explicit BTreeLeafMeta(uint32_t t_page_num)
        : PageMixin{t_page_num}, m_n_rows{0}, m_first_free{DEFAULT_FREE_OFF} {}
    BTreeLeafMeta(uint32_t t_page_num, uint16_t t_n_rows, uint16_t t_first_free)
        : PageMixin{t_page_num}, m_n_rows{t_n_rows},
          m_first_free{t_first_free} {}

  private:
    // offset 1: number of rows stored inside this leaf.
    uint16_t m_n_rows;
    // offset 3: the first free offset.
    //   default to 5, since 4 is the last byte of the metadata chunk.
    uint16_t m_first_free;
    static constexpr uint16_t DEFAULT_FREE_OFF = 5;

    friend void read_from_impl(BTreeLeafMeta& t_meta, std::istream& t_in);
    friend void write_to_impl(const BTreeLeafMeta& t_meta, std::ostream& t_out);
};

class BTreeInternalMeta : public PageMixin {
  public:
    explicit BTreeInternalMeta(uint32_t t_page_num)
        : PageMixin{t_page_num}, m_n_keys{0}, m_first_free{DEFAULT_FREE_OFF} {}
    BTreeInternalMeta(uint32_t t_page_num, uint16_t t_n_keys,
                      uint16_t t_first_free)
        : PageMixin{t_page_num}, m_n_keys{t_n_keys},
          m_first_free{t_first_free} {}

  private:
    // offset 1: number of keys currently stored inside this internal node.
    uint16_t m_n_keys;
    // offset 3: the first free offset.
    //   default to 5, since 4 is the last byte of the metadata chunk.
    uint16_t m_first_free;
    static constexpr uint16_t DEFAULT_FREE_OFF = 5;

    friend void read_from_impl(BTreeInternalMeta& t_meta, std::istream& t_in);
    friend void write_to_impl(const BTreeInternalMeta& t_meta,
                              std::ostream& t_out);
};

class HeapMeta : public PageMixin {
  public:
    explicit HeapMeta(uint32_t t_page_num)
        : PageMixin{t_page_num}, m_next_pg{0}, m_first_free{DEFAULT_FREE_OFF} {}
    HeapMeta(uint32_t t_page_num, uint32_t t_next_pg, uint16_t t_first_free)
        : PageMixin{t_page_num}, m_next_pg{t_next_pg},
          m_first_free{t_first_free} {}

    [[nodiscard]] constexpr auto get_next_pg() const -> uint32_t {
        return m_next_pg;
    }

    [[nodiscard]] constexpr auto get_first_free_off() const -> uint16_t {
        return m_first_free;
    }

    // this class is getting messy. Cleanup needed probably.
    // Given its name, it should only really be a "metadata holder", not the
    // full-blown (de)allocator.

    /**
     * @param t_off The offset from the start of this page.
     * @param t_in The file/stream this HeapMeta is written in.
     * @return The raw bytes of the fragment at the specified offset.
     * @details The data on how many bytes there are to read is contained inside
     * a fragment's header.
     * - The assumption is the end-user doesn't deal with this stuff, especially
     * the stream. Let's assume that the stream we passed in is the correct one.
     */
    auto get_raw_bytes_at(std::istream& t_in,
                          uint16_t t_off) const -> std::vector<std::byte>;

    /**
     * @class HeapPointer
     * @brief It's a pointer.
     */
    class HeapPointer {
      public:
        /**
         * @return The page number this heap pointer points to.
         */
        [[nodiscard]] constexpr auto get_pg_num() const noexcept -> uint32_t {
            return m_pg_num;
        }

        /**
         * @return The offset relative to the page number being pointed to.
         */
        [[nodiscard]] constexpr auto
        get_local_offset() const noexcept -> uint32_t {
            return m_off;
        }

        /**
         * @return The absolute offset value.
         */
        [[nodiscard]] constexpr auto
        get_total_offset() const noexcept -> uint64_t {
            return static_cast<uint64_t>(m_pg_num) *
                   static_cast<uint64_t>(m_off);
        }

        [[nodiscard]] constexpr auto is_valid() -> bool;

      private:
        constexpr HeapPointer() = default;
        constexpr HeapPointer(uint32_t t_pg_num, uint16_t t_off)
            : m_pg_num{t_pg_num}, m_off{t_off} {}
        // offset 0: 4-byte page pointer.
        uint32_t m_pg_num;
        // offset 4: 2-byte relative offset pointer.
        uint16_t m_off;
        friend class HeapMeta;
    };
    static_assert(std::is_trivial_v<HeapPointer>);

    /**
     * @class AllocRetValue
     * @brief Return value of HeapMeta::allocate
     *
     */
    struct AllocRetVal {
        // Pointer to the allocated space.
        HeapPointer ptr;
        // The amount allocated.
        // Since a page is assumed to be 4096 bytes (holds true for all your
        // computers probably), we only need an uint16 here.
        uint16_t alloc_size;
    };

    /**
     * @brief Allocates as much memory as possible until either we receive
     * enough bytes, or the page doesn't have enough space.
     * NOT IMPLEMENTED YET.
     *
     * @param t_io The file/stream this HeapMeta is written in.
     * @param t_max_siz The maximum size needed.
     */
    auto allocate(std::iostream& t_io, uint16_t t_max_siz) -> AllocRetVal;

    /**
     * @brief Deallocates the memory fragment at the specified offset.
     * NOT IMPLEMENTED YET.
     *
     * @param t_io
     * @param t_off
     */
    void deallocate(std::iostream& t_io, HeapPointer t_ptr);

  private:
    // offset 1: pointer to the next heap page.
    //   Default to 0, which means this heap page is the last one.
    //   In that case, if a new heap page is needed, a new heap page will be
    //   allocated from the free list.
    uint32_t m_next_pg;
    // offset 5: the first free fragment.
    //   By default, the first free fragment is at offset 7.
    //   And, there's always one fragment at offset 7.
    uint16_t m_first_free;
    static constexpr uint16_t DEFAULT_FREE_OFF = 7;

    friend void read_from_impl(HeapMeta& t_meta, std::istream& t_in);
    friend void write_to_impl(const HeapMeta& t_meta, std::ostream& t_out);
};

} // namespace tinydb::dbfile

#endif // !TINYDB_DBFILE_PAGE_HXX
