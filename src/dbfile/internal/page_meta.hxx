#ifndef TINYDB_DBFILE_INTERNAL_PAGE_HXX
#define TINYDB_DBFILE_INTERNAL_PAGE_HXX

#include "general/modules.hxx"
#include <cstdint>
#ifndef ENABLE_MODULE
#include "dbfile/internal/page_base.hxx"
#include <iosfwd>
#include <vector>
#endif // !ENABLE_MODULE

TINYDB_EXPORT
namespace tinydb::dbfile::internal {

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

    [[nodiscard]] constexpr auto get_next_pg() const noexcept -> uint32_t {
        return m_next_pg;
    }

  private:
    // offset 1: pointer to next page. Set to 0 if this is the last free page.
    //   If set to 0, the next free page should be the page right below this
    //   page in memory order.
    uint32_t m_next_pg;

};
void write_to(const FreePageMeta& t_meta, std::ostream& t_out);

class BTreeLeafMeta : public PageMixin {
  public:
    explicit BTreeLeafMeta(uint32_t t_page_num)
        : PageMixin{t_page_num}, m_n_rows{0}, m_first_free{DEFAULT_FREE_OFF} {}
    BTreeLeafMeta(uint32_t t_page_num, uint16_t t_n_rows, uint16_t t_first_free)
        : PageMixin{t_page_num}, m_n_rows{t_n_rows},
          m_first_free{t_first_free} {}

    [[nodiscard]] auto get_n_rows() const noexcept -> uint16_t {
        return m_n_rows;
    }

    [[nodiscard]] auto get_first_free() const noexcept -> uint16_t {
        return m_first_free;
    }

  private:
    // offset 1: number of rows stored inside this leaf.
    uint16_t m_n_rows;
    // offset 3: the first free offset.
    //   default to 5, since 4 is the last byte of the metadata chunk.
    uint16_t m_first_free;
    static constexpr uint16_t DEFAULT_FREE_OFF = 5;

};
void write_to(const BTreeLeafMeta& t_meta, std::ostream& t_out);

class BTreeInternalMeta : public PageMixin {
  public:
    explicit BTreeInternalMeta(uint32_t t_page_num)
        : PageMixin{t_page_num}, m_n_keys{0}, m_first_free{DEFAULT_FREE_OFF} {}
    BTreeInternalMeta(uint32_t t_page_num, uint16_t t_n_keys,
                      uint16_t t_first_free)
        : PageMixin{t_page_num}, m_n_keys{t_n_keys},
          m_first_free{t_first_free} {}

    [[nodiscard]] constexpr auto get_n_keys() const noexcept -> uint16_t {
        return m_n_keys;
    }

    [[nodiscard]] constexpr auto get_first_free_off() const noexcept -> uint16_t {
        return m_first_free;
    }

  private:
    // offset 1: number of keys currently stored inside this internal node.
    uint16_t m_n_keys;
    // offset 3: the first free offset.
    //   default to 5, since 4 is the last byte of the metadata chunk.
    uint16_t m_first_free;
    static constexpr uint16_t DEFAULT_FREE_OFF = 5;

};
void write_to(const BTreeInternalMeta& t_meta, std::ostream& t_out);

class HeapMeta : public PageMixin {
  public:
    explicit HeapMeta(uint32_t t_page_num)
        : PageMixin{t_page_num}, m_next_pg{0}, m_first_free{DEFAULT_FREE_OFF} {}
    HeapMeta(uint32_t t_page_num, uint32_t t_next_pg, uint16_t t_first_free)
        : PageMixin{t_page_num}, m_next_pg{t_next_pg},
          m_first_free{t_first_free} {}

    [[nodiscard]] constexpr auto get_next_pg() const noexcept -> uint32_t {
        return m_next_pg;
    }

    [[nodiscard]] constexpr auto
    get_first_free_off() const noexcept -> uint16_t {
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

    friend void read_from(HeapMeta& t_meta, std::istream& t_in);
    friend void write_to(const HeapMeta& t_meta, std::ostream& t_out);
    /**
     * @brief Allocates as much memory as possible until either we receive
     * enough bytes, or the page doesn't have enough space.
     * NOT IMPLEMENTED YET.
     *
     * @param t_io The file/stream this HeapMeta is written in.
     * @param t_max_siz The maximum size needed.
     */
    friend auto allocate(HeapMeta& t_meta, std::iostream& t_io,
                         uint16_t t_max_siz) -> AllocRetVal;

    /**
     * @brief Deallocates the memory fragment at the specified offset.
     * NOT IMPLEMENTED YET.
     *
     * @param t_io
     * @param t_off
     */
    void deallocate(HeapMeta& t_meta, std::iostream& t_io, HeapPointer t_ptr);
};

}

#endif // !TINYDB_DBFILE_INTERNAL_PAGE_HXX
