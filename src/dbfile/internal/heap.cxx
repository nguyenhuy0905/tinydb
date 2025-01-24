#include "offsets.hxx"
#ifndef ENABLE_MODULE
#include "dbfile/internal/heap.hxx"
#include "general/sizes.hxx"
#include <bit>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#else
module;
#include "dbfile/coltype.hxx"
#include "general/sizes.hxx"
#include <cassert>
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
[[maybe_unused]]
constexpr uint8_t USED_FRAG_ID = 1;
// since we started at 2^4
[[maybe_unused]]
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

struct Heap::Fragment {
    enum class FragType : char { // char so that I don't have to cast.
        Free = 0,
        Used,
        Chained,
    };
    // offset 0: 1-byte fragment type:
    //   - 0 if free.
    //   - 1 if used.
    //   - 2 if used AND contains a pointer to another heap.
    //     - To deal with super-duper large data.
    // offset 1: 2-byte size.
    //   - Remember that a page can only be 4096 bytes long.

    // pointer to the start of the header.
    // Not written into the database file.
    Ptr pos;
    page_off_t size;
    FragType type;
    static constexpr page_off_t HEADER_SIZE = sizeof(type) + sizeof(size);
};

void Heap::write_frag_to(const Heap::Fragment& t_frag, std::ostream& t_out) {
    t_out.seekp(t_frag.pos.pagenum * SIZEOF_PAGE + t_frag.pos.offset);
    auto& rdbuf = *t_out.rdbuf();
    // type
    rdbuf.sputc(static_cast<std::underlying_type_t<decltype(t_frag.type)>>(
        t_frag.type));
    // ptr
    write_ptr_to(Ptr{.pagenum = t_frag.pos.pagenum,
                     .offset = static_cast<page_off_t>(t_frag.pos.offset +
                                                       sizeof(t_frag.type))},
                 t_frag.pos, t_out);
    // size
    t_out.seekp(static_cast<std::streamoff>(t_frag.pos.pagenum * SIZEOF_PAGE +
                                            t_frag.pos.offset +
                                            // Ptr in-memory is 8 bytes due to
                                            // alignment, but we only need 6.
                                            sizeof(t_frag.type) + Ptr::SIZE));
    rdbuf.sputn(std::bit_cast<const char*>(&t_frag.size), sizeof(t_frag.size));
}

auto Heap::read_frag_from(const Ptr& t_pos, std::istream& t_in)
    -> Heap::Fragment {
    t_in.seekg(t_pos.pagenum * SIZEOF_PAGE + t_pos.offset);
    auto& rdbuf = *t_in.rdbuf();

    Heap::Fragment::FragType type{
        static_cast<std::underlying_type_t<Heap::Fragment::FragType>>(
            rdbuf.sbumpc())};

    auto ptr =
        read_ptr_from(Ptr{.pagenum = t_pos.pagenum,
                          .offset = static_cast<page_off_t>(t_pos.offset + 1)},
                      t_in);
    t_in.seekg(
        static_cast<std::streamoff>(t_pos.pagenum * SIZEOF_PAGE + t_pos.offset +
                                    sizeof(Heap::Fragment::type) + Ptr::SIZE));

    page_off_t size{};
    rdbuf.sgetn(std::bit_cast<char*>(&size), sizeof(size));

    return {.pos = ptr, .size = size, .type = type};
}

void write_heap_to(const Heap& t_heap, std::ostream& t_out) {
    t_out.seekp(HEAP_OFF);
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&t_heap.m_first_heap_page),
                         sizeof(t_heap.m_first_heap_page));
}

auto read_heap_from(std::istream& t_in) -> Heap {
    t_in.seekg(HEAP_OFF);
    page_ptr_t first_heap{};
    t_in.rdbuf()->sgetn(std::bit_cast<char*>(&first_heap), sizeof(first_heap));
    return Heap{first_heap};
}

// TODO: write Heap::malloc, then Heap::free
auto Heap::malloc(page_off_t t_size, FreeList& t_fl, std::iostream& t_io)
    -> Ptr {}

} // namespace tinydb::dbfile::internal
