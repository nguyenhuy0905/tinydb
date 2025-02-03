/**
 * @file heap_base.hxx
 * @brief Defines structures that the heap will use. See heap.
 *
 * Defines the structures of Ptr and Fragment. A Ptr is, well, a pointer to a
 * position in the file. A Fragment represents a memory fragment inside a heap
 * page.
 */

#ifndef TINYDB_DBFILE_INTERNAL_HEAP_BASE_HXX
#define TINYDB_DBFILE_INTERNAL_HEAP_BASE_HXX

#include "general/modules.hxx"
#ifndef ENABLE_MODULES
#include "dbfile/internal/page_base.hxx"
#include <cassert>
#include <utility>
#include <variant>
#endif // !ENABLE_MODULES

namespace tinydb::dbfile::internal {

/**
 * @class Ptr
 * @brief Points to some position. That's why it's named a pointer, duh.
 *
 */
TINYDB_EXPORT
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
TINYDB_EXPORT
[[nodiscard]] constexpr auto operator==(const Ptr& lhs, const Ptr& rhs)
    -> bool {
  return (lhs.pagenum == rhs.pagenum) && (lhs.offset == rhs.offset);
}

/**
 * @brief Bad.
 */
TINYDB_EXPORT
constexpr Ptr NullPtr{.pagenum = 0, .offset = 0};

/**
 * @class Fragment
 * @brief The in-memory representation of a fragment.
 *
 * Default-construct a `Fragment` returns an invalid fragment.
 *
 */
TINYDB_EXPORT
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

} // namespace tinydb::dbfile::internal

#endif // !TINYDB_DBFILE_INTERNAL_HEAP_BASE_HXX
