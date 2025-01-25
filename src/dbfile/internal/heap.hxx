#ifndef TINYDB_DBFILE_INTERNAL_HEAP_HXX
#define TINYDB_DBFILE_INTERNAL_HEAP_HXX

#include "modules.hxx"
#ifndef ENABLE_MODULE
#include "dbfile/internal/freelist.hxx"
#include "dbfile/internal/page_base.hxx"
#include <iosfwd>
#include <utility>
#include <variant>
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
    /**
     * @class Fragment
     * @brief The in-memory representation of a fragment.
     *
     */
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
        Ptr pos;
        FragExtra extra;
        page_off_t size;
        FragType type;
        static constexpr page_off_t NULL_FRAG_PTR = 0;
        // Default header size. All fragment types have headers at least this
        // size.
        static constexpr page_off_t HEADER_SIZE = sizeof(type) + sizeof(size);
        static constexpr page_off_t USED_FRAG_HEADER_SIZE = HEADER_SIZE;
        static constexpr page_off_t FREE_FRAG_HEADER_SIZE =
            HEADER_SIZE + sizeof(FreeFragExtra::next);
        static constexpr page_off_t CHAINED_FRAG_HEADER_SIZE =
            HEADER_SIZE + sizeof(Ptr::SIZE);
    };
    static_assert(std::is_trivially_copy_assignable_v<Fragment>);
    static_assert(std::is_trivially_copy_constructible_v<Fragment>);

    Heap() = default;
    explicit Heap(page_ptr_t t_first_heap_pg)
        : m_first_heap_page{t_first_heap_pg} {}
    /**
     * @brief Allocates a large enough chunk of memory. t_size must be smaller
     * than or equal to 4096 - HeapMeta::DEFAULT_FREE_OFF (which, at the moment
     * of writing the documentation, is 4081).
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
     * `return_value.second` larger than `return_value.first.pos.offset`
     */
    [[nodiscard]] auto malloc(page_off_t t_size, bool is_chained,
                              FreeList& t_fl, std::iostream& t_io)
        -> std::pair<Fragment, page_off_t>;

    /**
     * @brief Manual call to chain 2 `Fragment`s together. Both of these must be
     * of type `Chained`.
     *
     * @param t_to_chain
     * @param t_next_frag
     * @param t_io
     */
    void chain(Fragment& t_to_chain, const Fragment& t_next_frag,
               std::iostream& t_io);
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
