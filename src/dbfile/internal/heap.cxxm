module;
#include "general/sizes.hxx"
#include "general/offsets.hxx"
#include <bit>
#include <cassert>
#include <cmath>
#include <iostream>
#include <utility>
#include <variant>
#include <type_traits>
export module tinydb.dbfile.internal.heap;
import tinydb.dbfile.coltype;
import tinydb.dbfile.internal.page;
import tinydb.dbfile.internal.freelist;

namespace tinydb::dbfile::internal {

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

/**
 * @brief Gets the pointer at the specified position.
 *
 * @param t_pos The pointer to the position to read the pointer.
 * @param t_in The stream to read from.
 */
export auto read_ptr_from(const Ptr& t_pos, std::istream& t_in) -> Ptr {
    t_in.seekg(t_pos.pagenum * SIZEOF_PAGE + t_pos.offset);
    auto& rdbuf = *t_in.rdbuf();
    Ptr ret{};
    rdbuf.sgetn(std::bit_cast<char*>(&ret.pagenum), sizeof(ret.pagenum));
    rdbuf.sgetn(std::bit_cast<char*>(&ret.offset), sizeof(ret.offset));
    return ret;
}

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
 * @class Heap
 * @brief "Generic allocator," you may say. As opposed to the nice-and-efficient
 * block allocator that is FreeList. I know, naming scheme is a bit scuffed.
 *
 */
export class Heap {
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
            HEADER_SIZE + Ptr::SIZE;
    };
    static_assert(std::is_trivially_copy_assignable_v<Fragment>);
    static_assert(std::is_trivially_copy_constructible_v<Fragment>);

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
        // TODO: break down this giant function.
        // There are a few things that this function is doing:
        //   - Find a suitable heap page (a heap page whose largest fragment is
        //   large enough to accomodate the requested size).
        //   - Find the first suitable fragment in that page.
        //   - If the fragment is large, break the parts we don't use into a new
        //   free fragment.
        //   - And finally, write all the things that changed back to the
        //   stream.

        // TODO: when a fragment is chained, we need some extra size.
        assert(actual_size <= SIZEOF_PAGE - HeapMeta::DEFAULT_FREE_OFF);
        constexpr auto get_free_extra =
            [](Fragment& t_frag) -> Fragment::FreeFragExtra& {
            assert(std::get_if<Fragment::FreeFragExtra>(&t_frag.extra) !=
                   nullptr);
            return std::get<Fragment::FreeFragExtra>(t_frag.extra);
        };
        auto [heap_meta, max_pair, min_pair] = find_first_fit_heap_pg(
            actual_size - Fragment::USED_FRAG_HEADER_SIZE, t_fl, t_io);
        auto pagenum = heap_meta.get_pg_num();

        auto [ret_frag, next_frag, prev_frag, ret_off] = find_first_fit_frag(
            actual_size - Fragment::USED_FRAG_HEADER_SIZE, is_chained, max_pair,
            min_pair, heap_meta, t_io);

        // if the returned fragment still has room for another free fragment,
        // break it down.
        auto new_frag_size = [&]() {
            auto ret = ret_frag.size - t_size - Fragment::FREE_FRAG_HEADER_SIZE;
            if (ret > 0) {
                return static_cast<page_off_t>(ret);
            }
            return static_cast<page_off_t>(0);
        }();
        if (new_frag_size > 0) {
            // if there's a bug, try to add 1 to the function below.
            auto new_frag_off =
                static_cast<page_off_t>(ret_frag.pos.offset + actual_size);
            auto new_frag = Fragment{
                .pos{.pagenum = pagenum, .offset = new_frag_off},
                .extra{
                    Fragment::FreeFragExtra{.next = Fragment::NULL_FRAG_PTR}},
                // basically, we only care about the size of an used fragment.
                .size = static_cast<page_off_t>(
                    new_frag_size + (Fragment::FREE_FRAG_HEADER_SIZE -
                                     Fragment::USED_FRAG_HEADER_SIZE)),
                .type = Fragment::FragType::Free};
            ret_frag.size = t_size;
            // Nicer name for checking invalid fragment than the raw
            // null-pointer thingy I guess.
            constexpr auto is_invalid_frag = [](const Fragment& t_frag) {
                return t_frag.pos == NullPtr;
            };
            // Then the neighboring fragment, particularly the next fragment.
            if (!is_invalid_frag(next_frag)) {
                assert(std::get_if<Fragment::FreeFragExtra>(&next_frag.extra) !=
                       nullptr);
                get_free_extra(new_frag).next = next_frag.pos.offset;
            } else {
                next_frag = new_frag;
            }
            // then the heap page
            if (heap_meta.get_first_free_off() == ret_frag.pos.offset) {
                heap_meta.update_first_free(new_frag_off);
            }

            write_frag_to(new_frag, t_io);
        } else if (heap_meta.get_first_free_off() == ret_frag.pos.offset) {
            heap_meta.update_first_free(Fragment::NULL_FRAG_PTR);
        }
        // NOTE: read the following and the code above this if you're
        // implementing Heap::free:
        //
        // else, that bit of memory is lost to the entire system if I
        // change ret_frag.size. So I won't. Hopefully the API consumer won't
        // change anything. But to be fair, the only API consumer of this is,
        // me, even if my library got some users.

        write_frag_to(ret_frag, t_io);
        auto curr_update_frag = next_frag;
        // Need to force update if the return fragment is the min and/or the max
        // fragment.
        if (max_pair.second == curr_update_frag.pos.offset) {
            max_pair = {static_cast<page_off_t>(0), static_cast<page_off_t>(0)};
        }
        if (min_pair.second > curr_update_frag.pos.offset) {
            min_pair = {SIZEOF_PAGE, static_cast<page_off_t>(0)};
        }
        // Traverse the entire page to see which free fragments remaining are
        // the biggest and smallest.
        assert(std::get_if<Fragment::FreeFragExtra>(&curr_update_frag.extra) !=
               nullptr);
        for (auto o = heap_meta.get_first_free_off();
             o != Heap::Fragment::NULL_FRAG_PTR;
             o = get_free_extra(curr_update_frag).next) {
            curr_update_frag =
                read_frag_from(Ptr{.pagenum = pagenum, .offset = o}, t_io);

            if (max_pair.first < curr_update_frag.size) {
                max_pair = {curr_update_frag.size, curr_update_frag.pos.offset};
            }
            if (min_pair.first > curr_update_frag.size) {
                min_pair = {curr_update_frag.size, curr_update_frag.pos.offset};
            }
            assert(std::get_if<Fragment::FreeFragExtra>(
                       &curr_update_frag.extra) != nullptr);
        }
        // if there's no update, set min_pair to {0, 0} also.
        if (min_pair.first == SIZEOF_PAGE) {
            min_pair.first = 0;
        }

        heap_meta.update_min_pair(min_pair.first, min_pair.second);
        heap_meta.update_max_pair(max_pair.first, max_pair.second);
        write_to(heap_meta, t_io);
        // in case m_first_heap_page is updated
        write_heap_to(*this, t_io);

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
     * @brief Frees the memory pointed to by the pointer, and writes the pointer
     * to NullPtr.
     * @details This function also tries to coalesce free fragments. So, if you
     * are simply reassigning value of the pointer, and care about squeezing out
     * more performance, don't call this function. A "realloc" function will
     * probably be written after this has been implemented to accomodate that.
     *
     * NOT YET IMPLEMENTED.
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

    // malloc and free helpers

    struct FindHeapRetVal {
        HeapMeta heap_pg;
        std::pair<page_off_t, page_off_t> max_pair;
        std::pair<page_off_t, page_off_t> min_pair;
    };

    /**
     * @brief Finds the first heap page whose maximum fragment size is large
     * enough to accomodate an allocation of size t_size. If no such heap page
     * is found, a new heap page is requested from t_fl. If a new heap page is
     * requested, t_fl will initialize the allocated heap page and write the
     * page info into the stream t_io.
     *
     * TODO: document heap helper functions
     *
     * @param t_size
     * @param t_fl
     * @param t_io
     * @return
     */
    auto find_first_fit_heap_pg(page_off_t t_size, FreeList& t_fl,
                                std::iostream& t_io) -> FindHeapRetVal {
        assert(t_size <= SIZEOF_PAGE - HeapMeta::DEFAULT_FREE_OFF);
        auto alloc_new_heap_pg = [&]() {
            auto ret = t_fl.allocate_page<HeapMeta>(t_io);
            // initialize the only fragment, which is 4081 bytes long (including
            // the header stuff).
            Fragment write_to{.pos{.pagenum = ret.get_pg_num(),
                                   .offset = HeapMeta::DEFAULT_FREE_OFF},
                              .extra{Fragment::FreeFragExtra{
                                  .next = Heap::Fragment::NULL_FRAG_PTR}},
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
        return {
            .heap_pg = heap_meta, .max_pair = max_pair, .min_pair = min_pair};
    }
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
     *
     * @details Since min_pair and max_pair may be in an indeterminate state
     * here, meaning we can't really call update_min_pair or update_max_pair
     * without an error, we need to take their pairs by reference.
     *
     * And would you look at that. The function signature is ugly as fuck.
     *
     * @param t_size The allocation size.
     * @param is_chained
     * @param t_max_pair
     * @param t_min_pair
     * @param t_meta
     * @param t_io
     * @return
     */
    auto find_first_fit_frag(page_off_t t_size, bool is_chained,
                             std::pair<page_off_t, page_off_t>& t_max_pair,
                             std::pair<page_off_t, page_off_t>& t_min_pair,
                             HeapMeta& t_meta, std::istream& t_io)
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

        constexpr auto get_free_extra =
            [](Fragment& t_frag) -> Fragment::FreeFragExtra& {
            assert(std::get_if<Fragment::FreeFragExtra>(&t_frag.extra) !=
                   nullptr);
            return std::get<Fragment::FreeFragExtra>(t_frag.extra);
        };
        auto ret_frag = read_frag_from(
            Ptr{.pagenum = pagenum, .offset = t_meta.get_first_free_off()},
            t_io);
        Heap::Fragment prev_frag{.pos = NullPtr,
                                 .extra = Heap::Fragment::FreeFragExtra{},
                                 .size = 0,
                                 .type = Heap::Fragment::FragType::Free};
        while (ret_frag.size < t_size) {
            assert(std::get_if<Heap::Fragment::FreeFragExtra>(
                       &ret_frag.extra) != nullptr);
            assert(get_free_extra(ret_frag).next !=
                   Heap::Fragment::NULL_FRAG_PTR);

            prev_frag = ret_frag;
            ret_frag =
                read_frag_from(Ptr{.pagenum = pagenum,
                                   .offset = get_free_extra(ret_frag).next},
                               t_io);
        }
        // from this point, ret_frag's position will not be modified.
        assert(std::get_if<Fragment::FreeFragExtra>(&ret_frag.extra) !=
               nullptr);
        // set max and min to some placeholder values.
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
        if (get_free_extra(ret_frag).next != Fragment::NULL_FRAG_PTR) {
            next_frag =
                read_frag_from(Ptr{.pagenum = pagenum,
                                   .offset = get_free_extra(ret_frag).next},
                               t_io);
        }
        // Nicer name for checking invalid fragment than the raw null-pointer
        // thingy I guess.
        constexpr auto is_invalid_frag = [](const Fragment& t_frag) {
            return t_frag.pos == NullPtr;
        };

        if (!is_invalid_frag(prev_frag)) {
            auto next_off = get_free_extra(ret_frag).next;
            if (next_off != Fragment::NULL_FRAG_PTR) {
                next_frag = read_frag_from(
                    Ptr{.pagenum = pagenum, .offset = next_off}, t_io);
            }
        }

        // in case the fragment we allocated is the first free fragment.
        if (t_meta.get_first_free_off() == ret_frag.pos.offset) {
            if (!is_invalid_frag(next_frag)) {
                t_meta.update_first_free(get_free_extra(next_frag).next);
            }
        }
        get_free_extra(ret_frag).next = next_frag.pos.offset;
        // if the user needs to chain the fragments, they must have another
        // `Chained` fragment ready.
        auto ret_off = Heap::Fragment::USED_FRAG_HEADER_SIZE;

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

    friend void write_heap_to(const Heap& t_heap, std::ostream& t_out) {
        t_out.seekp(HEAP_OFF);
        t_out.rdbuf()->sputn(
            std::bit_cast<const char*>(&t_heap.m_first_heap_page),
            sizeof(t_heap.m_first_heap_page));
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

auto write_ptr_to(const Ptr& t_pos, const Ptr& t_ptr, std::ostream& t_out) {
    t_out.seekp(t_pos.pagenum * SIZEOF_PAGE + t_pos.offset);
    auto& rdbuf = *t_out.rdbuf();
    rdbuf.sputn(std::bit_cast<const char*>(&t_ptr.pagenum),
                sizeof(t_ptr.pagenum));
    rdbuf.sputn(std::bit_cast<const char*>(&t_ptr.offset),
                sizeof(t_ptr.offset));
}

void Heap::write_frag_to(const Heap::Fragment& t_frag, std::ostream& t_out) {
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

auto Heap::read_frag_from(const Ptr& t_pos, std::istream& t_in) -> Fragment {
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

} // namespace tinydb::dbfile::internal
