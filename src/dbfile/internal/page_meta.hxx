#ifndef TINYDB_DBFILE_INTERNAL_PAGE_HXX
#define TINYDB_DBFILE_INTERNAL_PAGE_HXX

#include "sizes.hxx"
#include "general/modules.hxx"
#include <cstdint>
#ifndef ENABLE_MODULE
#include "dbfile/internal/page_base.hxx"
#include <iosfwd>
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
    static constexpr page_off_t DEFAULT_FREE_OFF = 7;
    explicit HeapMeta(page_ptr_t t_page_num)
        : PageMixin{t_page_num}, m_next_pg{0}, m_first_free{DEFAULT_FREE_OFF} {}
    HeapMeta(page_ptr_t t_page_num, page_ptr_t t_next_pg,
             page_off_t t_first_free)
        : PageMixin{t_page_num}, m_next_pg{t_next_pg},
          m_first_free{t_first_free} {}

    [[nodiscard]] constexpr auto get_next_pg() const noexcept -> page_ptr_t {
        return m_next_pg;
    }

    [[nodiscard]] constexpr auto get_first_free_off() const noexcept
        -> page_off_t {
        return m_first_free;
    }

    constexpr auto update_first_free(page_off_t t_val) {
        if(t_val < DEFAULT_FREE_OFF || t_val > SIZEOF_PAGE) {
            // TODO: throw something more useful.
            throw 1;
        }
        m_first_free = t_val;
    }

  private:
    // offset 1: pointer to the next heap page.
    //   Default to 0, which means this heap page is the last one.
    //   In that case, if a new heap page is needed, a new heap page will be
    //   allocated from the free list.
    page_ptr_t m_next_pg;
    // offset 5: the first free fragment.
    //   By default, the first free fragment is at offset 7.
    //   And, there's always one fragment at offset 7.
    page_off_t m_first_free;
};

void write_to(const HeapMeta& t_meta, std::ostream& t_out);

} // namespace tinydb::dbfile::internal

#endif // !TINYDB_DBFILE_INTERNAL_PAGE_HXX
