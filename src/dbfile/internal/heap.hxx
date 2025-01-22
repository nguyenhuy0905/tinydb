#ifndef TINYDB_DBFILE_INTERNAL_HEAP_HXX
#define TINYDB_DBFILE_INTERNAL_HEAP_HXX

#include "modules.hxx"
#ifndef ENABLE_MODULE
#include "dbfile/internal/page_base.hxx"
#include <array>
#include <iosfwd>
#include <span>
#include <utility>
#endif // !ENABLE_MODULE

TINYDB_EXPORT
namespace tinydb::dbfile::internal {

/**
 * @class Ptr
 * @brief Points to some position. That's why it's named a pointer, duh.
 *
 */
struct Ptr {
    // offset 0: 4-byte page pointer.
    // offset 4: 2-byte offset relative to the start of the page.
    //   - The offset points to AFTER the required header of a fragment, which
    //   is 5 bytes in size.
    // offset 6: 2-byte size-of fragment, excluding the header stuff.
    // The header is at least 5 bytes, a fragment is at least 16 bytes. So size
    // is at least 11 bytes.

    page_ptr_t pagenum;
    // the offset points to offset 5 of a fragment, since that's where the
    // non-header data starts.
    page_off_t offset;
};

/**
 * @brief Gets the pointer at the specified position.
 *
 * @param t_pos The pointer to the position to read the pointer.
 * @param t_in The stream to read from.
 */
auto read_ptr_from(const Ptr& t_pos, std::istream& t_in) -> Ptr;

/**
 * @brief Useful for null-checking I suppose?
 *
 * @return If 2 AllocPtrs are the same.
 */
[[nodiscard]] auto operator==(const Ptr& lhs, const Ptr& rhs) -> bool {
    return (lhs.pagenum == rhs.pagenum) && (lhs.offset == rhs.offset);
}

/**
 * @brief Bad.
 */
static constexpr Ptr NullPtr{.pagenum = 0, .offset = 0};

/**
 * @class Heap
 * @brief "Generic allocator," you may say. As opposed to the nice-and-efficient
 * block allocator that is FreeList. I know, naming scheme is a bit scuffed.
 *
 */
class Heap {
  private:
    static constexpr uint8_t SIZEOF_BIN = 8;

  public:
    Heap() = default;

    /**
     * @brief Reads the data from a stream and returns the Heap.
     *
     * @param t_in The stream to read from.
     */
    static auto do_read_from(std::istream& t_in) -> Heap;

    [[nodiscard]] constexpr auto get_bins() noexcept
        -> std::span<Ptr, SIZEOF_BIN> {
        return std::span<Ptr, SIZEOF_BIN>{m_bins.begin(), SIZEOF_BIN};
    }

    /**
     * @brief Allocates a large enough chunk of memory. t_size must be smaller
     * than or equal to 2048.
     *
     * @param t_size Size of allocation.
     * @param t_io The stream to deal with.
     */
    [[nodiscard]] auto malloc(page_off_t t_size, std::iostream& t_io) -> Ptr;
    /**
     * @brief Frees the memory pointed to by the pointer, and writes the pointer
     * to NullPtr.
     *
     * @param t_ptr Pointer to memory to be freed.
     * @param t_io The stream to deal with.
     */
    void free(Ptr& t_ptr, std::iostream& t_io);

  private:
    // 16 (2^4), 32, 64, all the way to 2048 (2^11)
    explicit Heap(std::span<Ptr, SIZEOF_BIN> t_bins) noexcept
        // I hate this, but here is the TL;DR:
        //   - index_sequence simply creates an,
        //   well, index sequence. In this case, from 0 to 7
        //   (make_index_sequence<8>).
        //
        //   - The variadic template expands, in
        //   particular, t_bins[Idx]... expands into t_bins[0], t_bins[1],...,
        //   t_bins[7].
        //
        //   - Sticky note to myself:
        //   https://en.cppreference.com/w/cpp/utility/integer_sequence
        : m_bins{[&]<std::size_t... Idx>(std::index_sequence<Idx...>) {
              return std::array<Ptr, SIZEOF_BIN>{{t_bins[Idx]...}};
          }(std::make_index_sequence<SIZEOF_BIN>())} {}
    // offset 0 to 63: 8 pointers to memory fragments of specified sizes. These
    // fragments are memory-aligned.
    //   - If you read the comment on how "allocation is faster" with the
    //   artificially forced memory alignment, this is kind of the answer.
    //   Instead of needing up to, say, a couple thousand bin sizes, we can
    //   limit to 8.
    std::array<Ptr, SIZEOF_BIN> m_bins;

    static_assert(std::is_trivial_v<std::array<Ptr, SIZEOF_BIN>>);
};
// I wonder if all the bins in Heap are garbage with the default
// constructor.
static_assert(std::is_trivial_v<Heap>);

} // namespace tinydb::dbfile::internal

#endif // !TINYDB_DBFILE_INTERNAL_HEAP_HXX
