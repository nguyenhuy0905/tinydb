#ifndef TINYDB_DBFILE_INTERNAL_HEAP_HXX
#define TINYDB_DBFILE_INTERNAL_HEAP_HXX

#include "general/modules.hxx"
#ifndef ENABLE_MODULES
#include "dbfile/coltype.hxx"
#include "dbfile/internal/freelist.hxx"
#include "dbfile/internal/page_meta.hxx"
#include "dbfile/internal/page_serialize.hxx"
#include "general/offsets.hxx"
#include "general/sizes.hxx"
#include <cassert>
#include <bit>
#include <cmath>
#include <iosfwd>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#endif // !ENABLE_MODULES

namespace tinydb::dbfile::internal {

/**
 * @class Ptr
 * @brief Points to some position. That's why it's named a pointer, duh.
 *
 */
EXPORT
struct Ptr {
  // offset 0: 4-byte page pointer.
  // offset 4: 2-byte offset relative to the start of the page.
  // The position is thus (pagenum * SIZEOF_PAGE) + offset

  page_ptr_t pagenum;
  page_off_t offset;
  static constexpr page_off_t SIZE{sizeof(pagenum) + sizeof(offset)};
};

static_assert(std::is_trivial_v<Ptr>);

/**
 * @brief Useful for null-checking I suppose?
 *
 * @return If 2 AllocPtrs are the same.
 */
EXPORT
[[nodiscard]] constexpr auto operator==(const Ptr& lhs, const Ptr& rhs)
    -> bool {
  return (lhs.pagenum == rhs.pagenum) && (lhs.offset == rhs.offset);
}

/**
 * @brief Bad.
 */
EXPORT
constexpr Ptr NullPtr{.pagenum = 0, .offset = 0};

/**
 * @class Fragment
 * @brief The in-memory representation of a fragment.
 *
 * Default-construct a `Fragment` returns an invalid fragment.
 *
 */
EXPORT
struct Fragment {
  enum class FragType : char { // char so that I don't have to cast.
    Free = 0,
    Used,
    Chained,
  };
  // All the `Extra` types here mean extra stuff that each fragment type
  // inserts after the required fragment header.

  struct FreeFragExtra {
    page_off_t next;
  };
  struct ChainedFragExtra {
    Ptr next;
  };
  struct UsedFragExtra {};
  using FragExtra =
      std::variant<FreeFragExtra, UsedFragExtra, ChainedFragExtra>;
  // offset 0: 1-byte fragment type:
  //   - 0 if free.
  //   - 1 if used.
  //   - 2 if used **AND** contains a pointer to another heap.
  //     - To deal with super-duper large data.
  // offset 1: 2-byte size.
  //   - Remember that a page can only be 4096 bytes long.
  // If fragment type is used, there's nothing more.
  // If fragment type is chained:
  //   - offset 3: 6-byte pointer to the next fragment in the chain.
  //   NULL_FRAG_PTR if it's the last in chain.
  // If fragment type is free:
  //   - offset 3: 2-byte pointer to next fragment in the same page.

  // pointer to the start of the header.
  // Not written into the database file.
  Ptr pos{};
  FragExtra extra{FreeFragExtra{}};
  page_off_t size{};
  FragType type{FragType::Free};
  static constexpr page_off_t NULL_FRAG_PTR{0};
  // Default header size. All fragment types have headers at least this
  // size.
  static constexpr page_off_t HEADER_SIZE{sizeof(type) + sizeof(size)};
  static constexpr page_off_t USED_FRAG_HEADER_SIZE{HEADER_SIZE};
  static constexpr page_off_t FREE_FRAG_HEADER_SIZE{
      HEADER_SIZE + sizeof(FreeFragExtra::next)};
  static constexpr page_off_t CHAINED_FRAG_HEADER_SIZE{HEADER_SIZE + Ptr::SIZE};

  /**
   * @return The FreeFragExtra the fragment stores.
   * - Throws an exception if used on different types of fragments than Free.
   */
  constexpr auto get_free_extra() -> FreeFragExtra& {
    assert(std::get_if<FreeFragExtra>(&extra) != nullptr);
    return std::get<FreeFragExtra>(extra);
  }

  /**
   * @brief A fragment is invalid if its `pos` is pointing to a NullPtr.
   */
  [[nodiscard]] constexpr auto is_invalid() const noexcept -> bool {
    return pos == NullPtr;
  }

  [[nodiscard]] constexpr auto header_size() const noexcept -> page_off_t {
    switch (type) {
    case FragType::Free:
      return FREE_FRAG_HEADER_SIZE;
    case FragType::Used:
      return USED_FRAG_HEADER_SIZE;
    case FragType::Chained:
      return CHAINED_FRAG_HEADER_SIZE;
    default:
      std::unreachable();
    }
  }
};
static_assert(std::is_trivially_copy_assignable_v<Fragment>);
static_assert(std::is_trivially_copy_constructible_v<Fragment>);

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
EXPORT
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

}

#endif // !TINYDB_DBFILE_INTERNAL_HEAP_HXX
