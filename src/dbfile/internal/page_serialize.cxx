#ifndef ENABLE_MODULE
#include "dbfile/internal/page_serialize.hxx"
#include "dbfile/internal/page_impl.hxx"
#include "general/sizes.hxx"
#include <bit>
#include <iostream>
#else
module;
#include "general/sizes.hxx"
export module tinydb.dbfile.internal.page:serialize;
export import :base;
export import :impl;
import std;
#include "dbfile/internal/page_serialize.hxx"
#endif // !ENABLE_MODULE

TINYDB_EXPORT
namespace tinydb::dbfile::internal {

void write_to(const FreePageMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.get_pg_num() * SIZEOF_PAGE);
    t_out.rdbuf()->sputc(static_cast<pt_num>(PageType::Free));

    auto pgnum = t_meta.get_pg_num();
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&pgnum),
                         sizeof(pgnum));
}

void read_from(FreePageMeta& t_meta, std::istream& t_in) {
    t_in.seekg(t_meta.get_pg_num() * SIZEOF_PAGE);
    auto pagetype = static_cast<uint8_t>(t_in.rdbuf()->sbumpc());
    if (pagetype != static_cast<pt_num>(PageType::Free)) {
        // I should return something more meaningful here.
        return;
    }

    uint32_t next_pg{0};
    t_in.rdbuf()->sgetn(std::bit_cast<char*>(&next_pg),
                        sizeof(next_pg));
    t_meta = FreePageMeta{t_meta.get_pg_num(), pagetype};
}

void write_to(const BTreeLeafMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.get_pg_num() * SIZEOF_PAGE);
    t_out.rdbuf()->sputc(static_cast<pt_num>(PageType::BTreeLeaf));
    auto nrows = t_meta.get_n_rows();
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&nrows),
                         sizeof(nrows));
    auto first_free = t_meta.get_first_free();
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&first_free),
                         sizeof(first_free));
}

void read_from(BTreeLeafMeta& t_meta, std::istream& t_in) {
    t_in.seekg(t_meta.get_pg_num() * SIZEOF_PAGE);
    auto pagetype = static_cast<uint8_t>(t_in.rdbuf()->sbumpc());
    if (pagetype != static_cast<pt_num>(PageType::BTreeLeaf)) {
        // I should return something more meaningful here.
        return;
    }

    uint16_t nrows{0};
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&nrows),
                        sizeof(nrows));
    uint16_t first_free{0};
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&first_free),
                        sizeof(first_free));
    t_meta = BTreeLeafMeta{t_meta.get_pg_num(), nrows, first_free};
}

void write_to(const BTreeInternalMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.m_pg_num * SIZEOF_PAGE);
    t_out.rdbuf()->sputc(static_cast<pt_num>(PageType::BTreeInternal));
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&t_meta.m_n_keys),
                         sizeof(t_meta.m_n_keys));
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&t_meta.m_first_free),
                         sizeof(t_meta.m_first_free));
}

void read_from(BTreeInternalMeta& t_meta, std::istream& t_in) {
    t_in.seekg(t_meta.m_pg_num * SIZEOF_PAGE);
    auto pagetype = static_cast<uint8_t>(t_in.rdbuf()->sbumpc());
    if (pagetype != static_cast<pt_num>(PageType::BTreeInternal)) {
        // I should return something more meaningful here.
        return;
    }
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&t_meta.m_n_keys),
                        sizeof(t_meta.m_n_keys));
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&t_meta.m_first_free),
                        sizeof(t_meta.m_first_free));
}

// I should really reorganize these stuff
// NOLINTBEGIN

/**
 * @class HeapFragMeta
 * @brief Forms a free list inside a heap page.
 *
 */
struct HeapFragMeta {
    // offset 0: 2-byte, offset of previous heap fragment.
    // If there's no fragment before this, the value is nullopt, and is written
    // as a 0 on disk.
    // Next fragment is actually easy to get, given we know the size.
    std::optional<uint16_t> prev;
    // offset 2: 2-byte, size of this heap, not including the header.
    // If the offset of this heap plus size equals 4095, this is the last heap
    // fragment.
    uint16_t size;
    // offset 4: 1-byte, if this fragment is free.
    bool is_free;
};
// NOLINTEND

void write_to(const HeapMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.m_pg_num * SIZEOF_PAGE);
    t_out.rdbuf()->sputc(static_cast<pt_num>(PageType::Heap));
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&t_meta.m_next_pg),
                         sizeof(t_meta.m_next_pg));
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&t_meta.m_first_free),
                         sizeof(t_meta.m_first_free));
}

void read_from(HeapMeta& t_meta, std::istream& t_in) {
    t_in.seekg(t_meta.m_pg_num * SIZEOF_PAGE);
    auto pagetype = static_cast<uint8_t>(t_in.rdbuf()->sbumpc());
    if (pagetype != static_cast<pt_num>(PageType::Heap)) {
        // I should return something more meaningful here.
        return;
    }
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&t_meta.m_next_pg),
                        sizeof(t_meta.m_next_pg));
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&t_meta.m_first_free),
                        sizeof(t_meta.m_first_free));
}

} // namespace tinydb::dbfile
