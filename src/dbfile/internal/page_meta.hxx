#ifndef TINYDB_DBFILE_INTERNAL_PAGE_HXX
#define TINYDB_DBFILE_INTERNAL_PAGE_HXX

#include "modules.hxx"
#ifndef ENABLE_MODULE
#include "dbfile/internal/page_base.hxx"
#include "sizes.hxx"
#include <cassert>
#include <cstdint>
#include <iosfwd>
#include <type_traits>
#include <utility>
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
    explicit FreePageMeta(page_ptr_t t_page_num)
        : PageMixin{t_page_num}, m_next_pg{0} {}
    FreePageMeta(page_ptr_t t_page_num, page_ptr_t t_next_pg)
        : PageMixin{t_page_num}, m_next_pg{t_next_pg} {}

    [[nodiscard]] constexpr auto get_next_pg() const noexcept -> uint32_t {
        return m_next_pg;
    }

    constexpr auto update_next_pg(page_ptr_t t_next) noexcept {
        assert(t_next != NULL_PAGE);
        m_next_pg = t_next;
    }

  private:
    // offset 1: pointer to next page. Set to 0 if this is the last free page.
    //   If set to 0, the next free page should be the page right below this
    //   page in memory order.
    page_ptr_t m_next_pg;
};
void write_to(const FreePageMeta& t_meta, std::ostream& t_out);

class BTreeLeafMeta : public PageMixin {
  public:
    // number of rows
    using n_rows_t = uint16_t;

    explicit BTreeLeafMeta(page_ptr_t t_page_num)
        : PageMixin{t_page_num}, m_n_rows{0}, m_first_free{DEFAULT_FREE_OFF} {}
    BTreeLeafMeta(page_ptr_t t_page_num, n_rows_t t_n_rows,
                  page_off_t t_first_free)
        : PageMixin{t_page_num}, m_n_rows{t_n_rows},
          m_first_free{t_first_free} {}

    [[nodiscard]] auto get_n_rows() const noexcept -> n_rows_t {
        return m_n_rows;
    }

    [[nodiscard]] auto get_first_free() const noexcept -> page_off_t {
        return m_first_free;
    }

  private:
    // offset 1: number of rows stored inside this leaf.
    n_rows_t m_n_rows;
    // offset 3: the first free offset.
    //   default to 5, since 4 is the last byte of the metadata chunk.
    page_off_t m_first_free;
    static constexpr page_off_t DEFAULT_FREE_OFF = 5;
};
void write_to(const BTreeLeafMeta& t_meta, std::ostream& t_out);

class BTreeInternalMeta : public PageMixin {
  public:
    static constexpr page_off_t DEFAULT_FREE_OFF = 5;
    // number of keys
    using n_keys_t = uint16_t;

    explicit BTreeInternalMeta(page_ptr_t t_page_num)
        : PageMixin{t_page_num}, m_n_keys{0}, m_first_free{DEFAULT_FREE_OFF} {}
    BTreeInternalMeta(page_ptr_t t_page_num, n_keys_t t_n_keys,
                      page_off_t t_first_free)
        : PageMixin{t_page_num}, m_n_keys{t_n_keys},
          m_first_free{t_first_free} {}

    [[nodiscard]] constexpr auto get_n_keys() const noexcept -> n_keys_t {
        return m_n_keys;
    }

    [[nodiscard]] constexpr auto get_first_free_off() const noexcept
        -> page_off_t {
        return m_first_free;
    }

  private:
    // offset 1: number of keys currently stored inside this internal node.
    n_keys_t m_n_keys;
    // offset 3: the first free offset.
    //   default to 5, since 4 is the last byte of the metadata chunk.
    page_off_t m_first_free;
};
void write_to(const BTreeInternalMeta& t_meta, std::ostream& t_out);

class HeapMeta : public PageMixin {
  public:
    static constexpr page_off_t DEFAULT_FREE_OFF = 15;
    explicit HeapMeta(page_ptr_t t_page_num)
        : PageMixin{t_page_num}, m_next_pg{0}, m_first_free{DEFAULT_FREE_OFF},
          m_min{
              std::make_pair(SIZEOF_PAGE - DEFAULT_FREE_OFF, DEFAULT_FREE_OFF)},
          m_max{m_min} {}
    HeapMeta(page_ptr_t t_page_num, page_ptr_t t_next_pg,
             page_off_t t_first_free, std::pair<page_off_t, page_off_t> t_min,
             std::pair<page_off_t, page_off_t> t_max)
        : PageMixin{t_page_num}, m_next_pg{t_next_pg},
          m_first_free{t_first_free}, m_min{t_min}, m_max{t_max} {}

    [[nodiscard]] constexpr auto get_next_pg() const noexcept -> page_ptr_t {
        return m_next_pg;
    }

    [[nodiscard]] constexpr auto get_first_free_off() const noexcept
        -> page_off_t {
        return m_first_free;
    }

    [[nodiscard]] constexpr auto get_min_pair() const noexcept
        -> const std::pair<page_off_t, page_off_t>& {
        return m_min;
    }

    [[nodiscard]] constexpr auto get_max_pair() const noexcept
        -> const std::pair<page_off_t, page_off_t>& {
        return m_max;
    }

    constexpr auto update_next_pg(page_ptr_t t_next) noexcept {
        assert(t_next != NULL_PAGE);
        m_next_pg = t_next;
    }

    constexpr auto update_min_pair(page_off_t t_size, page_off_t t_off) {
        assert(t_size <= SIZEOF_PAGE - DEFAULT_FREE_OFF);
        assert(t_size <= m_max.first);
        m_min = std::make_pair(t_size, t_off);
    }

    constexpr auto update_max_pair(page_off_t t_size, page_off_t t_off) {
        assert(t_size <= SIZEOF_PAGE - DEFAULT_FREE_OFF);
        assert(t_size >= m_min.first);
        m_max = std::make_pair(t_size, t_off);
    }

    constexpr auto update_first_free(page_off_t t_val) {
        // Now I'm allowing something like 0 to be set here.

        if (t_val > 0 && (t_val < DEFAULT_FREE_OFF || t_val > SIZEOF_PAGE)) {
            // TODO: throw something more useful.
            throw 1;
        }
        m_first_free = t_val;
    }

  private:
    // offset 1: 4-byte pointer to the next heap page.
    //   Default to 0, which means this heap page is the last one.
    //   In that case, if a new heap page is needed, a new heap page will be
    //   allocated from the free list.
    page_ptr_t m_next_pg;
    // offset 5: 2-byte local offset to first free fragment.
    //   By default, the first free fragment is at offset 7.
    //   And, there's always one fragment at offset 7.
    page_off_t m_first_free;
    // offset 7: 2-byte minimum heap size.
    // offset 9: 2-byte minimum heap local offset.
    std::pair<page_off_t, page_off_t> m_min;
    // offset 11: 2-byte maximum heap size.
    // offset 13: 2-byte maximum heap local offset.
    std::pair<page_off_t, page_off_t> m_max;

    static_assert(!std::is_trivially_copyable_v<decltype(m_min)>);
};

void write_to(const HeapMeta& t_meta, std::ostream& t_out);

} // namespace tinydb::dbfile::internal

#endif // !TINYDB_DBFILE_INTERNAL_PAGE_HXX
