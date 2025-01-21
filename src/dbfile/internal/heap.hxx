#ifndef TINYDB_DBFILE_INTERNAL_HEAP_HXX
#define TINYDB_DBFILE_INTERNAL_HEAP_HXX

#include "modules.hxx"
#include <type_traits>
#ifndef ENABLE_MODULE
#include "dbfile/coltype.hxx"
#include "dbfile/internal/page_base.hxx"
#include "dbfile/internal/page_meta.hxx"
#include "general/sizes.hxx"
#include <compare>
#include <variant>
#include <vector>
#endif // !ENABLE_MODULE

TINYDB_EXPORT
namespace tinydb::dbfile::internal {

/**
 * @class AllocPtr
 * @brief Represents a pointer to a heap-allocated block.
 *
 */
struct AllocPtr {
    // the page number.
    page_ptr_t page;
    // the offset relative to the page.
    page_off_t offset;
    // the size of this pointer.
    page_off_t size;
};

/**
 * @brief Compares two `AllocPtr`s.
 *
 * @param l
 * @param r
 * @return
 */
[[nodiscard]] constexpr auto operator<=>(AllocPtr l, AllocPtr r) noexcept
    -> std::strong_ordering {
    if (l.page < r.page) {
        return std::strong_ordering::less;
    }
    if (l.page > r.page) {
        return std::strong_ordering::greater;
    }
    if (l.offset < r.offset) {
        return std::strong_ordering::less;
    }
    return std::strong_ordering::greater;
}

/**
 * @class ChainedAllocPtr
 * @brief A singly-linked list of fragment pointers.
 *
 */
struct ChainedAllocPtr {
    AllocPtr current;
    AllocPtr next;
};

static_assert(std::is_trivial_v<AllocPtr>);
static_assert(std::is_trivial_v<ChainedAllocPtr>);

using AllocPtrType = std::variant<AllocPtr, ChainedAllocPtr>;
using alloc_ptr_id_t = uint8_t;

/**
 * @param t_typ Type of alloc pointer.
 * @return Type ID of the alloc pointer.
 */
[[nodiscard]] constexpr auto get_ptr_type_id(const AllocPtrType& t_typ) noexcept
    -> alloc_ptr_id_t {
    // I dunno if I should be very consistent about reference vs value.
    // AllocPtr is 8 bytes (64 bits), which fits a "normal" pointer (void*).
    // ChainedAllocPtr contains 2 AllocPtrs, so it doesn't fit.
    // This is unlikely to gain me any performance, since this part of
    // the code is not a hot one.
    return std::visit(
        overload{[](AllocPtr) { return static_cast<alloc_ptr_id_t>(0); },
                 [](const ChainedAllocPtr&) {
                     return static_cast<alloc_ptr_id_t>(1);
                 }},
        t_typ);
}

/**
 * @class Heap
 * @brief Manages all the heap space.
 *
 */
class Heap {
  public:
    // TODO: heap and heap page.

    /**
     * @brief Allocates some amount of fragments.
     * @details
     *
     * @param t_size The total size to allocate.
     */
    auto allocate_big(uint64_t t_size) -> std::vector<ChainedAllocPtr>;
    /**
     * @brief Allocates one fragment.
     * @param t_size The size to allocate.
     */
    auto allocate_small(page_off_t t_size) -> AllocPtr;

    static constexpr page_off_t MAX_SMALL_ALLOC_SIZE =
        SIZEOF_PAGE - HeapMeta::DEFAULT_FREE_OFF;

  private:
    HeapMeta m_meta;
};

} // namespace tinydb::dbfile::internal

#endif // !TINYDB_DBFILE_INTERNAL_HEAP_HXX
