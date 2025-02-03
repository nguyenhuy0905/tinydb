#ifndef TINYDB_DBFILE_INTERNAL_PAGE_META_HXX
#define TINYDB_DBFILE_INTERNAL_PAGE_META_HXX

#ifndef ENABLE_MODULES
#include "dbfile/internal/heap_base.hxx"
#include "dbfile/internal/page_base.hxx"
#include "general/modules.hxx"
#include "general/sizes.hxx"
#include <cassert>
#include <cstdint>
#include <utility>
#endif // !ENABLE_MODULES

TINYDB_EXPORT
namespace tinydb::dbfile::internal {

/**
 * @class FreePageMeta
 * @brief Contains metadata about a free page
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
  // offset 0: 1 byte, equivalent to `PageType::Free`.
  // offset 1: pointer to next page. Set to 0 if this is the last free page.
  //   If set to 0, the next free page should be the page right below this
  //   page in memory order.
  page_ptr_t m_next_pg;
};

/**
 * @class BTreeLeafMeta
 * @brief Contains metadata about a BTree's leaf page.
 *
 */
class BTreeLeafMeta : public PageMixin {
public:
  // number of rows
  using n_rows_t = uint16_t;

  explicit BTreeLeafMeta(page_ptr_t t_page_num)
      : PageMixin{t_page_num}, m_n_rows{0}, m_first_free{DEFAULT_FREE_OFF} {}
  BTreeLeafMeta(page_ptr_t t_page_num, n_rows_t t_n_rows,
                page_off_t t_first_free)
      : PageMixin{t_page_num}, m_n_rows{t_n_rows}, m_first_free{t_first_free} {}

  [[nodiscard]] constexpr auto get_n_rows() const noexcept -> n_rows_t {
    return m_n_rows;
  }

  [[nodiscard]] constexpr auto get_first_free() const noexcept -> page_off_t {
    return m_first_free;
  }

private:
  // offset 0: 1 byte, equivalent to `PageType::BTreeLeaf`.
  // offset 1: 2 bytes, number of rows stored inside this leaf.
  n_rows_t m_n_rows;
  // offset 3: 2 bytes, the first free offset.
  //   default to 5, since 4 is the last byte of the metadata chunk.
  page_off_t m_first_free;
  static constexpr page_off_t DEFAULT_FREE_OFF =
      sizeof(m_n_rows) + sizeof(m_first_free);
};

/**
 * @class BTreeInternalMeta
 * @brief Contains metadata about a BTree's internal page.
 *
 */
class BTreeInternalMeta : public PageMixin {
public:
  // number of keys
  using n_keys_t = uint16_t;

private:
  // offset 0: 1 byte, equivalent to `PageType::BTreeInternal`.
  // offset 1: 2 bytes, number of keys currently stored inside this internal
  // node.
  n_keys_t m_n_keys;
  // offset 3: 2 bytes, the first free offset.
  //   default to 5, since 4 is the last byte of the metadata chunk.
  page_off_t m_first_free;

public:
  static constexpr page_off_t DEFAULT_FREE_OFF =
      sizeof(PageType) + sizeof(m_n_keys) + sizeof(m_first_free);

  /**
   * @brief The "placeholder" constructor. This is the same as an empty BTree
   * internal page.
   *
   * @param t_page_num The page number.
   */
  explicit BTreeInternalMeta(page_ptr_t t_page_num)
      : PageMixin{t_page_num}, m_n_keys{0}, m_first_free{DEFAULT_FREE_OFF} {}
  BTreeInternalMeta(page_ptr_t t_page_num, n_keys_t t_n_keys,
                    page_off_t t_first_free)
      : PageMixin{t_page_num}, m_n_keys{t_n_keys}, m_first_free{t_first_free} {}

  [[nodiscard]] constexpr auto get_n_keys() const noexcept -> n_keys_t {
    return m_n_keys;
  }

  [[nodiscard]] constexpr auto get_first_free_off() const noexcept
      -> page_off_t {
    return m_first_free;
  }
};

/**
 * @class HeapMeta
 * @brief Contains metadata about a heap page.
 *
 */
class HeapMeta : public PageMixin {
private:
  // offset 0: 1 byte, equivalent to `PageType::Heap`.
  // offset 1: 4 bytes, pointer to the next heap page.
  //   Default to 0, which means this heap page is the last one.
  //   In that case, if a new heap page is needed, a new heap page will be
  //   allocated from the free list.
  page_ptr_t m_next_pg;
  // offset 5: 4 bytes, pointer to the previous heap page.
  //   Default to 0, which means this heap page is the first one.
  page_ptr_t m_prev_pg;
  // offset 9: 2 bytes, offset to the first free fragment.
  page_off_t m_first_free;
  // offset 11: 2 bytes, maximum heap size.
  // offset 13: 2 bytes, maximum heap local offset.
  std::pair<page_off_t, page_off_t> m_max;

public:
  static constexpr page_off_t DEFAULT_FREE_OFF =
      sizeof(PageType) + sizeof(m_next_pg) + sizeof(m_prev_pg) +
      sizeof(m_first_free) + sizeof(m_max.first) + sizeof(m_max.second);
  explicit HeapMeta(page_ptr_t t_page_num)
      : PageMixin{t_page_num}, m_next_pg{NULL_PAGE}, m_prev_pg{NULL_PAGE},
        m_first_free{DEFAULT_FREE_OFF},
        m_max{std::make_pair(
            // the definition isn't available here: 5 is the size of a free
            // fragment header. You can imagine a heap page as a list of
            // different types of fragments.
            static_cast<page_off_t>(SIZEOF_PAGE - DEFAULT_FREE_OFF -
                                    Fragment::FREE_FRAG_HEADER_SIZE),
            static_cast<page_off_t>(DEFAULT_FREE_OFF))} {}
  HeapMeta(page_ptr_t t_page_num, page_ptr_t t_next_pg, page_ptr_t t_prev_pg,
           page_off_t t_first_free, std::pair<page_off_t, page_off_t> t_max)
      : PageMixin{t_page_num}, m_next_pg{t_next_pg}, m_prev_pg{t_prev_pg},
        m_first_free{t_first_free}, m_max{t_max} {}

  /**
   * @return The next page pointer.
   * Returns NULL_PAGE (aka. 0) if this page is the last page.
   */
  [[nodiscard]] constexpr auto get_next_pg() const noexcept -> page_ptr_t {
    return m_next_pg;
  }

  /**
   * @return The previous page pointer.
   * Returns NULL_PAGE (aka. 0) if this page is the first page.
   */
  [[nodiscard]] constexpr auto get_prev_pg() const noexcept -> page_ptr_t {
    return m_prev_pg;
  }

  /**
   * @return The first free offset
   * Points to Fragment::NULL_FRAG_PTR (aka, 0) if there's no free offset
   * left.
   */
  [[nodiscard]] constexpr auto get_first_free_off() const noexcept
      -> page_off_t {
    return m_first_free;
  }

  /**
   * @return The [size, offset] of the largest free fragment.
   * size == 0 means there's no free fragment inside.
   */
  [[nodiscard]] constexpr auto get_max_pair() const noexcept
      -> const std::pair<page_off_t, page_off_t>& {
    return m_max;
  }

  /**
   * @brief Updates the next page pointer
   *
   * Update to NULL_PAGE if this is the last page.
   *
   * @param t_next The next page pointer. Pass in NULL_PAGE if this is now the
   * last page.
   */
  constexpr auto update_next_pg(page_ptr_t t_next) noexcept {
    m_next_pg = t_next;
  }

  /**
   * @brief Updates the previous page pointer
   *
   * Update to NULL_PAGE if this is the first page.
   *
   * @param t_prev The previous page pointer. Pass in NULL_PAGE if this is now
   * the first page.
   */
  constexpr auto update_prev_pg(page_ptr_t t_prev) noexcept {
    m_prev_pg = t_prev;
  }

  /**
   * @brief Updates the maximum free fragment.
   *
   * Set `t_size` = 0 and `t_off` = 0 to indicate invalid pair.
   *
   * Otherwise, `t_size` must be smaller than `SIZEOF_PAGE - DEFAULT_FREE_OFF`,
   * and `t_off` must be smaller than `SIZEOF_PAGE`.
   *
   * TODO: move some non-heap stuff in the heap.cxx file into a new file.
   *
   * @param t_size New size
   * @param t_off new offset
   */
  constexpr auto update_max_pair(page_off_t t_size, page_off_t t_off) {
    assert((t_size == 0 && t_off == 0) ||
           (t_size <= SIZEOF_PAGE - DEFAULT_FREE_OFF -
                          Fragment::FREE_FRAG_HEADER_SIZE &&
            t_size > 0 && t_off < SIZEOF_PAGE));
    m_max = std::make_pair(t_size, t_off);
  }

  /**
   * @brief Updates the first free fragment pointer
   *
   * Set `t_val` to Fragment::NULL_FRAG_PTR (aka, 0) if there's no free fragment
   * left.
   * Otherwise, `t_val` must be larger than or equal to `DEFAULT_FREE_OFF` and
   * smaller than or equal to `SIZEOF_PAGE` (`DEFAULT_FREE_OFF <= t_val <=
   * SIZEOF_PAGE`).
   *
   * @param t_val The new offset.
   */
  constexpr auto update_first_free(page_off_t t_val) {
    // Now I'm allowing something like 0 to be set here.

    assert(t_val == 0 || (t_val >= DEFAULT_FREE_OFF && t_val <= SIZEOF_PAGE));
    m_first_free = t_val;
  }
};

} // namespace tinydb::dbfile::internal

#endif // !TINYDB_DBFILE_INTERNAL_PAGE_META_HXX
