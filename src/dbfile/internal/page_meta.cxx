module;
#include "general/sizes.hxx"
#include <cassert>
#ifndef IMPORT_STD
#include <cstdint>
#include <utility>
#endif
export module tinydb.dbfile.internal.page:meta;
export import :base;
#ifdef IMPORT_STD
import std;
#endif

export namespace tinydb::dbfile::internal {

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

class HeapMeta : public PageMixin {
  public:
    static constexpr page_off_t DEFAULT_FREE_OFF = 15;
    explicit HeapMeta(page_ptr_t t_page_num)
        : PageMixin{t_page_num}, m_next_pg{0}, m_prev_pg{0},
          m_first_free{DEFAULT_FREE_OFF},
          m_max{std::make_pair(
              static_cast<page_off_t>(SIZEOF_PAGE - DEFAULT_FREE_OFF),
              static_cast<page_off_t>(DEFAULT_FREE_OFF))} {}
    HeapMeta(page_ptr_t t_page_num, page_ptr_t t_next_pg, page_ptr_t t_prev_pg,
             page_off_t t_first_free, std::pair<page_off_t, page_off_t> t_max)
        : PageMixin{t_page_num}, m_next_pg{t_next_pg}, m_prev_pg{t_prev_pg},
          m_first_free{t_first_free}, m_max{t_max} {}

    [[nodiscard]] constexpr auto get_next_pg() const noexcept -> page_ptr_t {
        return m_next_pg;
    }

    [[nodiscard]] constexpr auto get_prev_pg() const noexcept -> page_ptr_t {
        return m_prev_pg;
    }

    [[nodiscard]] constexpr auto get_first_free_off() const noexcept
        -> page_off_t {
        return m_first_free;
    }

    [[nodiscard]] constexpr auto get_max_pair() const noexcept
        -> const std::pair<page_off_t, page_off_t>& {
        return m_max;
    }

    constexpr auto update_next_pg(page_ptr_t t_next) noexcept {
        m_next_pg = t_next;
    }

    constexpr auto update_prev_pg(page_ptr_t t_prev) noexcept {
        m_prev_pg = t_prev;
    }

    constexpr auto update_max_pair(page_off_t t_size, page_off_t t_off) {
        assert(t_size <= SIZEOF_PAGE - DEFAULT_FREE_OFF);
        m_max = std::make_pair(t_size, t_off);
    }

    constexpr auto update_first_free(page_off_t t_val) {
        // Now I'm allowing something like 0 to be set here.

        assert(t_val == 0 ||
               (t_val >= DEFAULT_FREE_OFF && t_val <= SIZEOF_PAGE));
        // if (t_val > 0 && (t_val < DEFAULT_FREE_OFF || t_val > SIZEOF_PAGE)) {
        //     // TODO: throw something more useful.
        //     throw 1;
        // }
        m_first_free = t_val;
    }

  private:
    // offset 1: 4-byte pointer to the next heap page.
    //   Default to 0, which means this heap page is the last one.
    //   In that case, if a new heap page is needed, a new heap page will be
    //   allocated from the free list.
    page_ptr_t m_next_pg;
    // offset 5: 4-byte pointer to the next heap page.
    //   Default to 0, which means this heap page is the last one.
    page_ptr_t m_prev_pg;
    // offset 9: offset to the first free fragment.
    page_off_t m_first_free;
    // offset 11: 2-byte maximum heap size.
    // offset 13: 2-byte maximum heap local offset.
    std::pair<page_off_t, page_off_t> m_max;
};

} // namespace tinydb::dbfile::internal
