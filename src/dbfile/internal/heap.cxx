module;
#include "general/offsets.hxx"
#include "general/sizes.hxx"
#include <cassert>
#ifndef IMPORT_STD
#include <bit>
#include <cmath>
#include <iostream>
#include <type_traits>
#include <utility>
#include <variant>
#endif
export module tinydb.dbfile.internal.heap;
import tinydb.dbfile.coltype;
import tinydb.dbfile.internal.page;
import tinydb.dbfile.internal.freelist;
#ifdef IMPORT_STD
import std;
#endif

namespace tinydb::dbfile::internal {

// TODO: move the non-export stuff down the file, again.
// I want to keep the interface more readable.

/**
 * @class Ptr
 * @brief Points to some position. That's why it's named a pointer, duh.
 *
 */
export struct Ptr {
    // offset 0: 4-byte page pointer.
    // offset 4: 2-byte offset relative to the start of the page.
    // The position is thus (pagenum * SIZEOF_PAGE) + offset

    page_ptr_t pagenum;
    page_off_t offset;
    static constexpr page_off_t SIZE = sizeof(pagenum) + sizeof(offset);
};

static_assert(std::is_trivial_v<Ptr>);

void write_ptr_to(const Ptr& t_pos, const Ptr& t_ptr,
                         std::ostream& t_out);

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
export [[nodiscard]] constexpr auto operator==(const Ptr& lhs, const Ptr& rhs)
    -> bool {
    return (lhs.pagenum == rhs.pagenum) && (lhs.offset == rhs.offset);
}

/**
 * @brief Bad.
 */
export constexpr Ptr NullPtr{.pagenum = 0, .offset = 0};

/**
 * @class Fragment
 * @brief The in-memory representation of a fragment.
 *
 */
export struct Fragment {
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
        HEADER_SIZE + Ptr::SIZE;

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
    constexpr auto is_invalid() -> bool { return pos == NullPtr; }
};
static_assert(std::is_trivially_copy_assignable_v<Fragment>);
static_assert(std::is_trivially_copy_constructible_v<Fragment>);

/**
 * @brief Writes the data inside the fragment into the stream.
 * This function performs no checking before writing data.
 *
 * @param t_frag The fragment.
 * @param t_out The write-only stream.
 */
void write_frag_to(const Fragment& t_frag, std::ostream& t_out);

/**
 * @brief Reads the fragment at the position pointed to from the stream.
 *
 * @param t_pos Pointer to the position of the fragment.
 * @param t_in The read-only stream.
 * @return The fragment read.
 *   - Exception if there's a read error.
 */
auto read_frag_from(const Ptr& t_pos, std::istream& t_in) -> Fragment;

struct FindFragRetVal {
    Fragment ret_frag;
    Fragment next;
    Fragment prev;
    page_off_t ret_off{};
};

/**
 * @brief Find the first-fit fragment. If the first-fit fragment is the
 * largest fragment in the page, update the max-pair of the page. Similarly
 * for smallest fragment and min-pair.
 *
 * @details Since min_pair and max_pair may be in an indeterminate state
 * before and after calling this function, meaning we can't really call
 * update_min_pair or update_max_pair without an error, we need to take their
 * pairs by reference.
 *
 * This function also formats the returned fragment's extra into the right
 * variant.
 *
 * // TODO: update the precondition of update_min_pair and update_max_pair.
 *
 * @param t_size The allocation size.
 * @param is_chained Whether the fragment to be allocated is chained.
 * @param t_max_pair First value must be larger than t_size (plus
 * CHAINED_FRAG_HEADER_SIZE if is_chained is true).
 * @param t_min_pair
 * @param t_meta
 * @param t_io
 * @return
 */
auto find_first_fit_frag(page_off_t t_size, bool is_chained,
                                std::pair<page_off_t, page_off_t>& t_max_pair,
                                std::pair<page_off_t, page_off_t>& t_min_pair,
                                const HeapMeta& t_meta, std::istream& t_io)
    -> FindFragRetVal;

/**
 * @brief Calculates the difference between the fragment's size and the size
 * requested. If the difference is large enough (larger than
 * FREE_FRAG_HEADER_SIZE), perform a fragmentation, creating a new free
 * fragment.
 *
 * @param t_old_frag The fragment to try break. This fragment will be allocated
 * to the user.
 * @param t_next_frag The next fragment of `t_old_frag`. Can be in an
 * indeterminate state.
 * @param t_old_frag_size The requested size.
 * @param heap_meta The heap meta of the heap page containing `t_old_frag` (and
 * consequently t_next_frag). Taken by reference. In case `t_old_frag` is the
 * first free fragment, update `heap_meta`'s first free.
 * @param t_io The stream to read/write.
 * @return True if a new fragment is created, false otherwise.
 */
auto try_break_frag(Fragment& t_old_frag, Fragment& t_next_frag,
                           page_off_t t_old_frag_size, HeapMeta& heap_meta,
                           std::iostream& t_io) -> bool;

/**
 * @brief Updates all the parameters passed in if needed before writing them
 * into the stream.
 *
 * @param t_heap_meta Needs to be updated if max_pair and min_pair changes.
 * @param t_next_frag Will not be updated. The updating is already done in
 * `try_break_frag.`
 * @param t_max_pair Updated if there's a larger fragment (not happening in
 * `malloc`), or if it's a .
 * @param t_min_pair Updated if there's a smaller fragment.
 * @param t_io The read/write stream.
 */
void update_write_heap_pg(HeapMeta& t_heap_meta,
                                 const Fragment& t_next_frag,
                                 std::pair<page_off_t, page_off_t>& t_max_pair,
                                 decltype(t_max_pair)& t_min_pair,
                                 std::iostream& t_io);

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
export class Heap {
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
    [[nodiscard]] auto malloc(page_off_t t_size, bool is_chained,
                              FreeList& t_fl, std::iostream& t_io)
        -> std::pair<Fragment, page_off_t> {
        // chained fragment is free fragment plus a pointer, so naturally it
        // needs Ptr::SIZE (6) more bytes.
        auto actual_size = static_cast<page_off_t>(
            (is_chained ? Fragment::CHAINED_FRAG_HEADER_SIZE
                        : Fragment::USED_FRAG_HEADER_SIZE) +
            t_size);

        assert(actual_size <= SIZEOF_PAGE - HeapMeta::DEFAULT_FREE_OFF);

        auto [heap_meta, max_pair, min_pair] = find_first_fit_heap_pg(
            actual_size - Fragment::USED_FRAG_HEADER_SIZE, t_fl, t_io);
        auto [ret_frag, next_frag, prev_frag, ret_off] = find_first_fit_frag(
            actual_size - Fragment::USED_FRAG_HEADER_SIZE, is_chained, max_pair,
            min_pair, heap_meta, t_io);

        try_break_frag(ret_frag, next_frag, t_size, heap_meta, t_io);
        write_frag_to(ret_frag, t_io);
        update_write_heap_pg(heap_meta, next_frag, max_pair, min_pair, t_io);

        return std::make_pair(ret_frag, ret_off);
    }

    /**
     * @brief Manual call to chain 2 `Fragment`s together. Both of these must be
     * of type `Chained`.
     *
     * @param t_to_chain
     * @param t_next_frag
     * @param t_out
     */
    void chain(Fragment& t_to_chain, const Fragment& t_next_frag,
               std::ostream& t_out) {
        assert(std::get_if<Fragment::ChainedFragExtra>(&t_to_chain.extra) !=
               nullptr);
        assert(std::get_if<Fragment::ChainedFragExtra>(&t_next_frag.extra) !=
               nullptr);
        std::get<Fragment::ChainedFragExtra>(t_to_chain.extra).next =
            t_next_frag.pos;
        write_frag_to(t_to_chain, t_out);
    }

    /**
     * @brief Releases the memory held by a fragment. Fragment must be of type
     * Used.
     *
     * NOT IMPLEMENTED YET.
     *
     * @param t_frag The fragment to free, must be of type Used.
     * @param t_fl In case the heap page the fragment points to is entirely
     * free. Then, that page is released to the freelist.
     * @param t_io The database read/write stream.
     *
     * @details
     */
    void free(Fragment& t_frag, FreeList& t_fl, std::iostream& t_io);

  private:
    // offset 0: 4-byte pointer to the first heap.
    page_ptr_t m_first_heap_page{NULL_PAGE};

    // malloc and free helpers

    struct FindHeapRetVal {
        HeapMeta heap_pg;
        std::pair<page_off_t, page_off_t> max_pair;
        std::pair<page_off_t, page_off_t> min_pair;
    };

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
    auto find_first_fit_heap_pg(page_off_t t_size, FreeList& t_fl,
                                std::iostream& t_io) -> FindHeapRetVal;

    void write_heap_to(std::ostream& t_out) {
        t_out.seekp(HEAP_OFF);
        t_out.rdbuf()->sputn(std::bit_cast<const char*>(&m_first_heap_page),
                             sizeof(m_first_heap_page));
    }
};

static_assert(!std::is_trivially_constructible_v<Heap>);

/**
 * @brief Reads from a stream and gets the heap.
 *
 * @param t_in The input stream.
 */
auto read_heap_from(std::istream& t_in) -> Heap {
    t_in.seekg(HEAP_OFF);
    page_ptr_t first_heap{};
    t_in.rdbuf()->sgetn(std::bit_cast<char*>(&first_heap), sizeof(first_heap));
    return Heap{first_heap};
}

void write_ptr_to(const Ptr& t_pos, const Ptr& t_ptr,
                         std::ostream& t_out) {
    t_out.seekp(t_pos.pagenum * SIZEOF_PAGE + t_pos.offset);
    auto& rdbuf = *t_out.rdbuf();
    rdbuf.sputn(std::bit_cast<const char*>(&t_ptr.pagenum),
                sizeof(t_ptr.pagenum));
    rdbuf.sputn(std::bit_cast<const char*>(&t_ptr.offset),
                sizeof(t_ptr.offset));
}

auto read_ptr_from(const Ptr& t_pos, std::istream& t_in) -> Ptr {
    t_in.seekg(t_pos.pagenum * SIZEOF_PAGE + t_pos.offset);
    auto& rdbuf = *t_in.rdbuf();
    Ptr ret{};
    rdbuf.sgetn(std::bit_cast<char*>(&ret.pagenum), sizeof(ret.pagenum));
    rdbuf.sgetn(std::bit_cast<char*>(&ret.offset), sizeof(ret.offset));
    return ret;
}

void write_frag_to(const Fragment& t_frag, std::ostream& t_out) {
    t_out.seekp(t_frag.pos.pagenum * SIZEOF_PAGE + t_frag.pos.offset);
    auto& rdbuf = *t_out.rdbuf();
    // type
    rdbuf.sputc(static_cast<std::underlying_type_t<decltype(t_frag.type)>>(
        t_frag.type));
    // size
    t_out.seekp(static_cast<std::streamoff>(t_frag.pos.pagenum * SIZEOF_PAGE +
                                            t_frag.pos.offset +
                                            // Ptr in-memory is 8 bytes due to
                                            // alignment, but we only need 6.
                                            sizeof(t_frag.type) + Ptr::SIZE));
    rdbuf.sputn(std::bit_cast<const char*>(&t_frag.size), sizeof(t_frag.size));
    std::visit(
        overload{[&](const Fragment::FreeFragExtra& t_free_extra) {
                     rdbuf.sputn(std::bit_cast<const char*>(&t_free_extra.next),
                                 sizeof(t_free_extra.next));
                 },
                 [&](const Fragment::UsedFragExtra&) {},
                 [&](const Fragment::ChainedFragExtra& t_chain_extra) {
                     write_ptr_to(
                         Ptr{.pagenum = t_frag.pos.pagenum,
                             .offset = static_cast<page_off_t>(
                                 t_frag.pos.offset + sizeof(t_frag.type) +
                                 sizeof(t_frag.size))},
                         t_chain_extra.next, t_out);
                 }},
        t_frag.extra);
}

auto read_frag_from(const Ptr& t_pos, std::istream& t_in) -> Fragment {
    t_in.seekg(t_pos.pagenum * SIZEOF_PAGE + t_pos.offset);
    auto& rdbuf = *t_in.rdbuf();

    Fragment::FragType type{
        static_cast<std::underlying_type_t<Fragment::FragType>>(
            rdbuf.sbumpc())};

    Ptr curr_pos{.pagenum = t_pos.pagenum,
                 .offset = static_cast<page_off_t>(
                     t_pos.offset + sizeof(Fragment::type) + Ptr::SIZE)};
    t_in.seekg(static_cast<std::streamoff>(curr_pos.pagenum * SIZEOF_PAGE +
                                           curr_pos.offset));

    page_off_t size{};
    rdbuf.sgetn(std::bit_cast<char*>(&size), sizeof(size));
    curr_pos.offset += sizeof(size);

    Fragment::FragExtra extra{Fragment::UsedFragExtra{}};
    switch (Fragment::FragType{type}) {
        using enum Fragment::FragType;
    case Free: {
        page_off_t next{};
        rdbuf.sgetn(std::bit_cast<char*>(&next), sizeof(next));
        extra = Fragment::FreeFragExtra{.next = next};
        break;
    }
    case Used:
        break;
    case Chained: {
        Ptr next = read_ptr_from(curr_pos, t_in);
        extra = Fragment::ChainedFragExtra{.next = next};
        break;
    }
    default:
        std::unreachable();
    }

    return {.pos{t_pos},
            .extra{extra},
            .size = size,
            .type = Fragment::FragType{type}};
}

auto find_first_fit_frag(page_off_t t_size, bool is_chained,
                                std::pair<page_off_t, page_off_t>& t_max_pair,
                                std::pair<page_off_t, page_off_t>& t_min_pair,
                                const HeapMeta& t_meta, std::istream& t_io)
    -> FindFragRetVal {
    // first-fit scheme. You know, a database is meant to be read from more
    // than update. If one wants frequent updating, he/she would probably
    // use a giant-hashtable type of database. If it's meant to be more
    // rarely updated, I can afford some fragmentation. Fragmentation is
    // only terrible in the case of "you have a bunch of heap-allocated
    // strings that are all relatively short (say, <1000 bytes) and their
    // sizes vary wildly". For giant strings that take one or more pages to
    // allocate, this isn't an issue really. The number of fragments over
    // the total amount of heap memory allocated is smaller.

    auto pagenum = t_meta.get_pg_num();

    auto ret_frag = read_frag_from(
        Ptr{.pagenum = pagenum, .offset = t_meta.get_first_free_off()}, t_io);
    Fragment prev_frag{.pos = NullPtr,
                       .extra = Fragment::FreeFragExtra{},
                       .size = 0,
                       .type = Fragment::FragType::Free};
    while (ret_frag.size < t_size) {
        assert(std::get_if<Fragment::FreeFragExtra>(&ret_frag.extra) !=
               nullptr);
        assert(ret_frag.get_free_extra().next != Fragment::NULL_FRAG_PTR);

        prev_frag = ret_frag;
        ret_frag = read_frag_from(
            Ptr{.pagenum = pagenum, .offset = ret_frag.get_free_extra().next},
            t_io);
    }
    // from this point, ret_frag's position will not be modified.
    assert(std::get_if<Fragment::FreeFragExtra>(&ret_frag.extra) != nullptr);
    // set max and min to some placeholder values, if max and min happens to
    // point to the same fragment that will be allocated.
    if (t_max_pair.second == ret_frag.pos.offset) {
        t_max_pair = {static_cast<page_off_t>(0), Fragment::NULL_FRAG_PTR};
    }
    if (t_min_pair.second == ret_frag.pos.offset) {
        t_min_pair = {SIZEOF_PAGE, Fragment::NULL_FRAG_PTR};
    }

    // record the 2 "neighbor" free fragments (neighbor in the sense of
    // "closest together" here) for update later. Of course, only update if
    // there exists those 2.
    Fragment next_frag = prev_frag;
    if (ret_frag.get_free_extra().next != Fragment::NULL_FRAG_PTR) {
        next_frag = read_frag_from(
            Ptr{.pagenum = pagenum, .offset = ret_frag.get_free_extra().next},
            t_io);
    }
    if (!prev_frag.is_invalid()) {
        auto next_off = ret_frag.get_free_extra().next;
        if (next_off != Fragment::NULL_FRAG_PTR) {
            next_frag = read_frag_from(
                Ptr{.pagenum = pagenum, .offset = next_off}, t_io);
        }
    }

    ret_frag.get_free_extra().next = next_frag.pos.offset;
    // if the user needs to chain the fragments, they must have another
    // `Chained` fragment ready.
    auto ret_off = Fragment::USED_FRAG_HEADER_SIZE;

    // if we have is_chained == true
    ret_frag.type = [&]() {
        if (is_chained) {
            ret_frag.extra = Fragment::ChainedFragExtra{NullPtr};
            ret_off = Fragment::CHAINED_FRAG_HEADER_SIZE;
            return Fragment::FragType::Chained;
        }
        ret_frag.extra = Fragment::UsedFragExtra{};
        return Fragment::FragType::Used;
    }();
    return {.ret_frag = ret_frag,
            .next = next_frag,
            .prev = prev_frag,
            .ret_off = ret_off};
}

auto Heap::find_first_fit_heap_pg(page_off_t t_size, FreeList& t_fl,
                                  std::iostream& t_io) -> FindHeapRetVal {
    assert(t_size <= SIZEOF_PAGE - HeapMeta::DEFAULT_FREE_OFF);
    auto alloc_new_heap_pg = [&]() {
        auto ret = t_fl.allocate_page<HeapMeta>(t_io);
        // initialize the only fragment, which is 4081 bytes long (including
        // the header stuff).
        Fragment write_to{
            .pos{.pagenum = ret.get_pg_num(),
                 .offset = HeapMeta::DEFAULT_FREE_OFF},
            .extra{Fragment::FreeFragExtra{.next = Fragment::NULL_FRAG_PTR}},
            // When this thing is used, it is converted into
            // an used fragment anyways.
            .size = SIZEOF_PAGE - HeapMeta::DEFAULT_FREE_OFF -
                    Fragment::USED_FRAG_HEADER_SIZE,
            .type = Fragment::FragType::Free};
        write_frag_to(write_to, t_io);
        return ret;
    };
    auto heap_meta = [&]() {
        if (m_first_heap_page == NULL_PAGE) {
            auto ret = alloc_new_heap_pg();
            m_first_heap_page = ret.get_pg_num();
            return ret;
        } else {
            return read_from<HeapMeta>(m_first_heap_page, t_io);
        }
    }();
    // search for the first heap page that has a large enough fragment.
    auto max_pair = heap_meta.get_max_pair();
    auto min_pair = heap_meta.get_min_pair();
    auto pagenum = heap_meta.get_pg_num();
    {
        // this one is to update the heap page later on
        // this name is terrible. I should have created a struct that has
        // fields size and offset.
        while (max_pair.first < t_size && pagenum != NULL_PAGE) {
            pagenum = heap_meta.get_next_pg();
            if (pagenum == NULL_PAGE) {
                break;
            }
            heap_meta = read_from<HeapMeta>(pagenum, t_io);
            max_pair = heap_meta.get_max_pair();
        }
        // if we don't even have a large enough fragment to use, ask the
        // freelist for a new heap page. Pretty much "inventing" `mmap()`
        // here.
        if (pagenum == NULL_PAGE) {
            auto prev_heap = heap_meta;
            heap_meta = alloc_new_heap_pg();
            pagenum = heap_meta.get_pg_num();
            prev_heap.update_next_pg(pagenum);
            write_to(prev_heap, t_io);
        }
    }
    return {.heap_pg = heap_meta, .max_pair = max_pair, .min_pair = min_pair};
}

auto try_break_frag(Fragment& t_old_frag, Fragment& t_next_frag,
                           page_off_t t_old_frag_size, HeapMeta& heap_meta,
                           std::iostream& t_io) -> bool {
    auto actual_size = [&]() {
        if (std::get_if<Fragment::UsedFragExtra>(&t_old_frag.extra) !=
            nullptr) {
            return t_old_frag_size + Fragment::USED_FRAG_HEADER_SIZE;
        }
        return t_old_frag_size + Fragment::CHAINED_FRAG_HEADER_SIZE;
    }();

    // if the returned fragment still has room for another free fragment,
    // break it down.
    auto new_frag_size = [&]() {
        auto ret =
            t_old_frag.size - t_old_frag_size - Fragment::FREE_FRAG_HEADER_SIZE;
        if (ret > 0) {
            return static_cast<page_off_t>(ret);
        }
        return static_cast<page_off_t>(0);
    }();
    if (new_frag_size > 0) {
        auto new_frag_off =
            static_cast<page_off_t>(t_old_frag.pos.offset + actual_size);
        auto new_frag = Fragment{
            .pos{.pagenum = t_old_frag.pos.pagenum, .offset = new_frag_off},
            .extra{Fragment::FreeFragExtra{.next = Fragment::NULL_FRAG_PTR}},
            // basically, we only care about the size of an used fragment.
            .size = static_cast<page_off_t>(new_frag_size +
                                            (Fragment::FREE_FRAG_HEADER_SIZE -
                                             Fragment::USED_FRAG_HEADER_SIZE)),
            .type = Fragment::FragType::Free};
        t_old_frag.size = t_old_frag_size;
        // Update the neighboring fragment, particularly the next fragment.
        if (!t_next_frag.is_invalid()) {
            assert(std::get_if<Fragment::FreeFragExtra>(&t_next_frag.extra) !=
                   nullptr);
            new_frag.get_free_extra().next = t_next_frag.pos.offset;
        } else {
            t_next_frag = new_frag;
        }
        // then the heap page
        if (heap_meta.get_first_free_off() == t_old_frag.pos.offset) {
            heap_meta.update_first_free(new_frag_off);
        }
        // TODO: move the writes back to malloc.

        write_frag_to(new_frag, t_io);
        return true;
    }
    if (heap_meta.get_first_free_off() == t_old_frag.pos.offset) {
        heap_meta.update_first_free(Fragment::NULL_FRAG_PTR);
    }
    return false;
}

void update_write_heap_pg(HeapMeta& heap_meta, const Fragment& next_frag,
                                 std::pair<page_off_t, page_off_t>& max_pair,
                                 decltype(max_pair)& min_pair,
                                 std::iostream& t_io) {
    auto pagenum = heap_meta.get_pg_num();
    auto curr_update_frag = next_frag;
    // Force update. I'm a bit paranoid.
    if (max_pair.second == curr_update_frag.pos.offset) {
        max_pair = {static_cast<page_off_t>(0), static_cast<page_off_t>(0)};
    }
    if (min_pair.second == curr_update_frag.pos.offset) {
        min_pair = {SIZEOF_PAGE, static_cast<page_off_t>(0)};
    }
    // Traverse the entire page to see which free fragments remaining are
    // the biggest and smallest.
    for (auto o = heap_meta.get_first_free_off(); o != Fragment::NULL_FRAG_PTR;
         o = curr_update_frag.get_free_extra().next) {
        assert(std::get_if<Fragment::FreeFragExtra>(&curr_update_frag.extra) !=
               nullptr);
        if (max_pair.first < curr_update_frag.size) {
            max_pair = {curr_update_frag.size, curr_update_frag.pos.offset};
        }
        if (min_pair.first > curr_update_frag.size) {
            min_pair = {curr_update_frag.size, curr_update_frag.pos.offset};
        }
        curr_update_frag =
            read_frag_from(Ptr{.pagenum = pagenum, .offset = o}, t_io);
    }
    // if min_pair is not updated at all, meaning there is no fragment left.
    // first = 0 is basically the same as "this page is out of memory".
    if (min_pair.first == SIZEOF_PAGE) {
        min_pair.first = 0;
        max_pair.first = 0;
    }

    heap_meta.update_min_pair(min_pair.first, min_pair.second);
    heap_meta.update_max_pair(max_pair.first, max_pair.second);
    write_to(heap_meta, t_io);
}

} // namespace tinydb::dbfile::internal
