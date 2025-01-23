#include "offsets.hxx"
#ifndef ENABLE_MODULE
#include "dbfile/coltype.hxx"
#include "dbfile/internal/freelist.hxx"
#include "dbfile/internal/heap.hxx"
#include "dbfile/internal/page_base.hxx"
#include "dbfile/internal/page_meta.hxx"
#include "general/sizes.hxx"
#include <bit>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <variant>
#else
module;
#include "dbfile/coltype.hxx"
#include "general/sizes.hxx"
#include <cstdint>
export module tinydb.dbfile.internal.heap;
import tinydb.dbfile.internal.page;
import tinydb.dbfile.internal.freelist;
import std;
#include "dbfile/internal/heap.hxx"
#endif // !ENABLE_MODULE

namespace {
[[maybe_unused]]
constexpr uint8_t FREE_FRAG_ID = 0;
constexpr uint8_t USED_FRAG_ID = 1;
// since we started at 2^4
constexpr uint8_t START_POW = 4;

} // namespace

namespace tinydb::dbfile::internal {

auto read_ptr_from(const Ptr& t_pos, std::istream& t_in) -> Ptr {
    t_in.seekg(t_pos.pagenum * SIZEOF_PAGE + t_pos.offset);
    auto& rdbuf = *t_in.rdbuf();
    Ptr ret{};
    rdbuf.sgetn(std::bit_cast<char*>(&ret.pagenum), sizeof(ret.pagenum));
    rdbuf.sgetn(std::bit_cast<char*>(&ret.offset), sizeof(ret.offset));
    return ret;
}

auto write_ptr_to(const Ptr& t_pos, const Ptr& t_ptr, std::ostream& t_out) {
    t_out.seekp(t_pos.pagenum * SIZEOF_PAGE + t_pos.offset);
    auto& rdbuf = *t_out.rdbuf();
    rdbuf.sputn(std::bit_cast<const char*>(&t_ptr.pagenum),
                sizeof(t_ptr.pagenum));
    rdbuf.sputn(std::bit_cast<const char*>(&t_ptr.offset),
                sizeof(t_ptr.offset));
}

// More stuff from the bins:
//   - The pointers in the bins are pointing to FreeFragments. Obviously.

struct FreeFragment {
    static constexpr page_off_t SIZE = 15;
    // offset 7: 8-byte AllocPtr.
    // chains into a linked list by Heap.
    // With that in mind, a fragment must be at least 15 bytes in size.
    // Memory alignment kicks in. Each fragment must be at least 16 bytes.
    //
    // By "memory alignment," I mean the fragments' sizes must be a power of 2.
    //
    // Technically memory alignment isn't necessary if we are dealing with
    // writing into streams and stuff here. You could say this is kind of
    // memory-wasteful, but it makes allocations faster. Typical time-space
    // tradeoff.
    Ptr next_frag;
};

struct UsedFragment {
    static constexpr page_off_t HEADER_SIZE = 7;
    // there's nothing here.
    // offset 7 until (7 + AllocPtr::size): the data.
    // For "large" data, that a fragment cannot fully keep,
    // another 8 bytes should be reserved for an AllocPtr.
};
static_assert(std::is_trivial_v<FreeFragment> &&
              std::is_trivial_v<UsedFragment>);

using FragType = std::variant<FreeFragment, UsedFragment>;

struct Fragment {
    // offset 0: 1-byte, fragment type:
    //   - 0 == FreeFragment
    //   - 1 == UsedFragment
    // offset 1: 2-byte local offset pointer to the previous fragment.
    // offset 3: 2-byte local offset pointer to the next fragment.
    // offset 5: 2-byte size of fragment.

    FragType frag;
    // naming is confusing here.
    // This forms a linked list with fragments INSIDE one page.
    // prev_local_frag is 0 when there isn't any fragment before this one in its
    // page.
    // next_local_frag is 0 when there isn't any fragment after this one in its
    // page.
    page_off_t prev_local_frag;
    page_off_t next_local_frag;
    page_off_t size;
    static constexpr page_off_t FRAGMENT_HEADER_SIZE = 7;
};

void write_frag_to(const Fragment& t_frag, const Ptr& t_ptr,
                   std::ostream& t_out) {
    t_out.seekp(t_ptr.pagenum * SIZEOF_PAGE + t_ptr.offset);
    auto& rdbuf = *t_out.rdbuf();
    rdbuf.sputc(std::visit(overload{[&](const FreeFragment&) { return '\0'; },
                                    [&](const UsedFragment&) { return '\1'; }},
                           t_frag.frag));
    rdbuf.sputn(std::bit_cast<const char*>(&t_frag.prev_local_frag),
                sizeof(t_frag.prev_local_frag));
    rdbuf.sputn(std::bit_cast<const char*>(&t_frag.next_local_frag),
                sizeof(t_frag.next_local_frag));
    rdbuf.sputn(std::bit_cast<const char*>(&t_frag.size), sizeof(t_frag.size));
    std::visit(overload{[&](const FreeFragment& frag) {
                            constexpr uint8_t DEFAULT_OFF = 7;
                            write_ptr_to(Ptr{.pagenum = t_ptr.pagenum,
                                             .offset = static_cast<page_off_t>(
                                                 t_ptr.offset + DEFAULT_OFF)},
                                         frag.next_frag, t_out);
                        },
                        [&](const UsedFragment&) {}},
               t_frag.frag);
}

// variant isn't trivial.
static_assert(!std::is_trivial_v<Fragment>);

/**
 * @brief Reads and returns a fragment at the pointed memory location.
 *
 * @param frag_pos The pointer to the memory location to read.
 * @param t_in The stream to read from.
 * @return The fragment read. Or throws an exception if I/O exception is
 * enabled.
 */
auto read_frag_from(const Ptr& frag_pos, std::istream& t_in) -> Fragment {
    t_in.seekg(frag_pos.pagenum * SIZEOF_PAGE + frag_pos.offset);
    auto ret = Fragment{};
    auto& rdbuf = *t_in.rdbuf();
    auto fragtype = static_cast<uint8_t>(rdbuf.sbumpc());
    // page_off_t prevlocal{0};
    rdbuf.sgetn(std::bit_cast<char*>(&ret.prev_local_frag),
                sizeof(ret.prev_local_frag));
    // page_off_t nextlocal{0};
    rdbuf.sgetn(std::bit_cast<char*>(&ret.next_local_frag),
                sizeof(ret.next_local_frag));
    rdbuf.sgetn(std::bit_cast<char*>(&ret.size), sizeof(ret.size));

    if (fragtype == USED_FRAG_ID) {
        ret.frag = UsedFragment{};
    } else {
        ret.frag = FreeFragment{.next_frag{read_ptr_from(
            Ptr{.pagenum = frag_pos.pagenum,
                .offset = static_cast<page_off_t>(
                    frag_pos.offset + Fragment::FRAGMENT_HEADER_SIZE)},
            t_in)}};
    }
    return ret;
}

void write_heap_to(const Heap& t_heap, std::ostream& t_out) {
    t_out.seekp(HEAP_OFF);
    std::span bins = t_heap.get_bins();
    Ptr write_pos{.pagenum = 0, .offset = HEAP_OFF};
    for (auto& ptr : bins) {
        write_ptr_to(write_pos, ptr, t_out);
        write_pos.offset += Ptr::SIZE;
    }
}

auto read_heap_from(std::istream& t_in) -> Heap {
    t_in.seekg(HEAP_OFF);
    Ptr pos{.pagenum = 0, .offset = HEAP_OFF};
    std::array<Ptr, Heap::SIZEOF_BIN> arr{};
    for (uint8_t i = 0; i < Heap::SIZEOF_BIN; ++i) {
        arr.at(i) = read_ptr_from(pos, t_in);
        pos.offset += Ptr::SIZE;
    }
    return Heap{arr.begin(), Heap::SIZEOF_BIN};
}

auto Heap::malloc(page_off_t t_size, FreeList& t_fl, std::iostream& t_io)
    -> Ptr {
    if (t_size > SIZEOF_PAGE / 2 || t_size < std::pow(2, 4)) {
        return NullPtr;
    }

    auto bin_num = static_cast<uint8_t>(std::log2(t_size) + 1);
    Ptr ret = m_bins.at(bin_num);
    if (ret != NullPtr) {
        Fragment read_frag = read_frag_from(m_bins.at(bin_num), t_io);
        auto free_frag = std::get<FreeFragment>(read_frag.frag);
        write_frag_to(Fragment{.frag = UsedFragment{},
                               .prev_local_frag = read_frag.prev_local_frag,
                               .next_local_frag = read_frag.next_local_frag,
                               .size = t_size},
                      m_bins.at(bin_num), t_io);
        m_bins.at(bin_num) = free_frag.next_frag;
        return ret;
    }
    // ret == NullPtr
    auto new_heap_pg = t_fl.allocate_page<HeapMeta>(t_io);
    auto new_frag = Fragment{
        .frag = UsedFragment{},
        .prev_local_frag = 0,
        .next_local_frag = 0,
        .size = static_cast<page_off_t>(std::pow(2, bin_num)),
    };
    auto ret_ptr = Ptr{.pagenum = new_heap_pg.get_pg_num(),
                       .offset = HeapMeta::DEFAULT_FREE_OFF};
    write_frag_to(new_frag, ret_ptr, t_io);
    // update the heap page.
    new_heap_pg.update_first_free(static_cast<page_off_t>(
        new_heap_pg.get_pg_num() + UsedFragment::HEADER_SIZE + new_frag.size));
    write_to(new_heap_pg, t_io);

    return ret_ptr;
}

// template <typename DirF>
//     requires requires(const Fragment frag, DirF f, page_off_t num) {
//         // basically either gives back frag.prev_local_frag or
//         // frag.next_local_frag.
//         num = f(frag);
//     }
// [[maybe_unused]]
// auto coalesce(DirF t_callable, Fragment& t_curr_free, const Ptr& t_pos,
//               std::iostream& t_io) -> Ptr {
//     // TODO: write a coalesce function.
//
//     return NullPtr;
// }
// auto coalesce_next(Fragment& t_curr_free, const Ptr& t_pos,
//                    std::iostream& t_io) {
//     return coalesce([](const Fragment& frag) { return frag.next_local_frag;
//     },
//                     t_curr_free, t_pos, t_io);
// }
//
// auto coalesce_prev(Fragment& t_curr_free, const Ptr& t_pos,
//                    std::iostream& t_io) {
//     return coalesce([](const Fragment& frag) { return frag.prev_local_frag;
//     },
//                     t_curr_free, t_pos, t_io);
// }

} // namespace tinydb::dbfile::internal
