#ifndef TINYDB_DBFILE_INTERNAL_HEAP_HXX
#define TINYDB_DBFILE_INTERNAL_HEAP_HXX

#include "modules.hxx"
#ifndef ENABLE_MODULE
#include "dbfile/internal/freelist.hxx"
#include "dbfile/internal/page_base.hxx"
#include "dbfile/internal/page_meta.hxx"
#include <iosfwd>
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

static_assert(std::is_trivial_v<Ptr>);

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
class Heap {
  public:
    Heap() = default;
    explicit Heap(page_ptr_t t_first_heap_pg)
        : m_first_heap_page{t_first_heap_pg} {}
    /**
     * @brief Allocates a large enough chunk of memory. t_size must be smaller
     * than or equal to 2048.
     *
     * @param t_size Size of allocation.
     * @param t_fl In case we need to allocate a new page.
     * @param t_io The stream to deal with.
     */
    [[nodiscard]] auto malloc(page_off_t t_size, FreeList& t_fl,
                              std::iostream& t_io) -> Ptr;
    /**
     * @brief Frees the memory pointed to by the pointer, and writes the pointer
     * to NullPtr.
     *
     * @param t_ptr Pointer to memory to be freed.
     * @param t_fl In case we need to deallocate a page.
     * @param t_io The stream to deal with.
     */
    void free(Ptr& t_ptr, FreeList& t_fl, std::iostream& t_io);

  private:
    // offset 0: 4-byte pointer to the first heap.
    page_ptr_t m_first_heap_page{0};

    struct Fragment;
    static void write_frag_to(const Fragment&, std::ostream&);
    static auto read_frag_from(const Ptr&, std::istream&) -> Fragment;

    friend void write_heap_to(const Heap& t_heap, std::ostream& t_out);
};

static_assert(!std::is_trivially_constructible_v<Heap>);

/**
 * @brief Reads from a stream and gets the heap.
 *
 * @param t_in The input stream.
 */
auto read_heap_from(std::istream& t_in) -> Heap;

} // namespace tinydb::dbfile::internal

#endif // !TINYDB_DBFILE_INTERNAL_HEAP_HXX
