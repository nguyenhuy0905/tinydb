#ifndef TINYDB_DBFILE_INTERNAL_HEAP_HXX
#define TINYDB_DBFILE_INTERNAL_HEAP_HXX

#include "modules.hxx"
#ifndef ENABLE_MODULE
#include "dbfile/internal/freelist.hxx"
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
    // The position is thus (pagenum * SIZEOF_PAGE) + offset

    page_ptr_t pagenum;
    page_off_t offset;
    static constexpr page_off_t SIZE = sizeof(pagenum) + sizeof(offset);
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
[[nodiscard]] constexpr auto operator==(const Ptr& lhs, const Ptr& rhs)
    -> bool {
    return (lhs.pagenum == rhs.pagenum) && (lhs.offset == rhs.offset);
}

/**
 * @brief Bad.
 */
constexpr Ptr NullPtr{.pagenum = 0, .offset = 0};

/**
 * @class Heap
 * @brief "Generic allocator," you may say. As opposed to the nice-and-efficient
 * block allocator that is FreeList. I know, naming scheme is a bit scuffed.
 *
 */
struct Heap {
    Heap() : m_bins{} {}
    // 16 (2^4), 32, 64, all the way to 4096 (2^12)
    static constexpr uint8_t SIZEOF_BIN = 9;

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

    template <typename... SpanArgs>
        requires requires(SpanArgs... args) {
            std::span<Ptr, SIZEOF_BIN>{args...};
        }
    explicit Heap(SpanArgs... args)
        : m_bins{[&]() {
              std::span<Ptr, SIZEOF_BIN> t_bins{args...};
              return [&]<std::size_t... Idx>(std::index_sequence<Idx...>) {
                  return std::array<Ptr, SIZEOF_BIN>{t_bins[Idx]...};
              }(std::make_index_sequence<SIZEOF_BIN>());
          }()} {}

    [[nodiscard]] constexpr auto get_bins() const noexcept
        -> std::span<const Ptr, SIZEOF_BIN> {
        return std::span<const Ptr, SIZEOF_BIN>{m_bins.begin(), SIZEOF_BIN};
    }

    /**
     * @brief Allocates a large enough chunk of memory. t_size must be smaller
     * than or equal to 2048.
     *
     * @param t_size Size of allocation.
     * @param t_io The stream to deal with.
     */
    [[nodiscard]] auto malloc(page_off_t t_size, FreeList& t_fl,
                              std::iostream& t_io) -> Ptr;
    /**
     * @brief Frees the memory pointed to by the pointer, and writes the pointer
     * to NullPtr.
     *
     * @param t_ptr Pointer to memory to be freed.
     * @param t_io The stream to deal with.
     */
    void free(Ptr& t_ptr, std::iostream& t_io);

    // offset 0 to 63: 8 pointers to memory fragments of specified sizes. These
    // fragments are memory-aligned.
    //   - If you read the comment on how "allocation is faster" with the
    //   artificially forced memory alignment, this is kind of the answer.
    //   Instead of needing up to, say, a couple thousand bin sizes, we can
    //   limit to 8.
    std::array<Ptr, SIZEOF_BIN> m_bins;

    static_assert(std::is_trivial_v<std::array<Ptr, SIZEOF_BIN>>);
};

/**
 * @brief Reads from a stream and gets the heap.
 *
 * @param t_in The input stream.
 */
auto read_heap_from(std::istream& t_in) -> Heap;

void write_heap_to(const Heap& t_heap, std::ostream& t_out);

} // namespace tinydb::dbfile::internal

#endif // !TINYDB_DBFILE_INTERNAL_HEAP_HXX
