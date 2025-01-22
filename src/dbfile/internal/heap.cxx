#ifndef ENABLE_MODULE
#include "dbfile/internal/heap.hxx"
#include "dbfile/internal/page_base.hxx"
#include "general/sizes.hxx"
#include <bit>
#include <cstdint>
#include <iostream>
#include <variant>
#else
module;
#include "general/sizes.hxx"
#include <cstdint>
export module tinydb.dbfile.internal:heap;
import tinydb.dbfile.internal.page:base;
import std;
#endif // !ENABLE_MODULE

namespace {
[[maybe_unused]]
constexpr uint8_t FREE_FRAG_ID = 0;
constexpr uint8_t USED_FRAG_ID = 1;
} // namespace

namespace tinydb::dbfile::internal {

auto read_ptr_from(const Ptr& t_ptr, std::istream& t_in) -> Ptr {
    t_in.seekg(t_ptr.pagenum * SIZEOF_PAGE + t_ptr.offset);
    auto& rdbuf = *t_in.rdbuf();
    Ptr ret{};
    rdbuf.sgetn(std::bit_cast<char*>(&ret.pagenum), sizeof(ret.pagenum));
    rdbuf.sgetn(std::bit_cast<char*>(&ret.offset), sizeof(ret.offset));
    return ret;
}

struct FreeFragment {
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

} // namespace tinydb::dbfile::internal
