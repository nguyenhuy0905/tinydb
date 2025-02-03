#ifndef TINYDB_DBFILE_INTERNAL_HEAP_HXX
#define TINYDB_DBFILE_INTERNAL_HEAP_HXX

#include "general/modules.hxx"
#ifndef ENABLE_MODULES
#include "dbfile/internal/freelist.hxx"
#include "dbfile/internal/heap_base.hxx"
#include "dbfile/internal/page_meta.hxx"
#include <cassert>
#include <cmath>
#include <iosfwd>
#include <type_traits>
#include <utility>
#include <variant>
#endif // !ENABLE_MODULES

namespace tinydb::dbfile::internal {

/**
 * @class Heap
 * @brief A freelist allocator.
 *
 * This allocator is created from 2 levels of linked lists. The first level is a
 * singly-linked list between the heap pages. Inside each heap page contains a
 * singly-linked list of memory fragments.
 * To find a new allocation, the program first traverses the list of heap pages
 * until it finds a page whose max fragment is large enough to accomodate the
 * requested size. Then, it traverses the inner linked list of the heap page
 * to find the first fragment that can accomodate the requested size.
 *
 */
TINYDB_EXPORT
class Heap {
public:
  Heap() = default;
  explicit Heap(page_ptr_t t_first_heap_pg)
      : m_first_heap_page{t_first_heap_pg} {}

  /**
   * @brief Allocates a large enough chunk of memory. t_size must be smaller
   * than or equal to 4096 - HeapMeta::DEFAULT_FREE_OFF -
   * USED_FRAG_HEADER_SIZE (which, at the moment of writing the documentation,
   * is 4076).
   * If is_chained is true, the maximum size is only 4070
   * (HeapMeta::DEFAULT_FREE_OFF - CHAINED_FRAG_HEADER_SIZE).
   *
   * @param t_size Size of allocation.
   * @param is_chained If you
   * @param t_fl In case we need to allocate a new page.
   * @param t_io The stream to deal with.
   * @return A pair:
   *   - The first value is the fragment allocated.
   *   - The second is the header size of the fragment. Data **MUST** be
   * written into the pointer whose `pagenum` is the same as
   * `return_value.first.pos.pagenum` and whose `offset` is
   * `return_value.second + return_value.first.pos.offset`
   */
  [[nodiscard]] auto malloc(page_off_t t_size, bool is_chained, FreeList& t_fl,
                            std::iostream& t_io)
      -> std::pair<Fragment, page_off_t>;

  /**
   * @brief Manual call to chain 2 `Fragment`s together. Both of these must be
   * of type `Chained`.
   *
   * @param t_to_chain
   * @param t_next_frag
   * @param t_out
   */
  static void chain(Fragment& t_to_chain, const Fragment& t_next_frag,
                    std::ostream& t_out);

  /**
   * @brief Releases the memory held by a fragment. Fragment must be of type
   * Used.
   *
   * COALESCE NOT IMPLEMENTED YET.
   *
   * @param t_frag The fragment to free, must be of type Used.
   * @param t_fl In case the heap page the fragment points to is entirely
   * free. Then, that page is released to the freelist.
   * @param t_io The database read/write stream.
   *
   * @details
   */
  void free(Fragment&& t_frag, [[maybe_unused]] FreeList& t_fl,
            std::iostream& t_io);

private:
  // offset 0: 4-byte pointer to the first heap.
  page_ptr_t m_first_heap_page{NULL_PAGE};

  // malloc and free helpers

  // struct FindHeapRetVal {
  //     HeapMeta heap_pg;
  //     std::pair<page_off_t, page_off_t> max_pair;
  // };

  /**
   * @brief Finds the first heap page whose maximum fragment size is large
   * enough to accomodate an allocation of size t_size.
   * If no such heap page is found, a new heap page is requested from t_fl. If
   * a new heap page is requested, t_fl will initialize the allocated heap
   * page and write the page info into the stream t_io.
   * If `m_first_heap_page` is NULL_PAGE before calling this function, it will
   * be updated.
   *
   * @param t_size The size to allocate.
   * @param t_fl The free list to (potentially) request a new page from.
   * @param t_io The read/write stream.
   * @return
   */
  auto find_first_fit_heap_pg(page_off_t t_size, bool is_chained,
                              FreeList& t_fl, std::iostream& t_io) -> HeapMeta;

  void write_heap_to(std::ostream& t_out);
};

} // namespace tinydb::dbfile::internal

#endif // !TINYDB_DBFILE_INTERNAL_HEAP_HXX
