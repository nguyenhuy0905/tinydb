#ifndef ENABLE_MODULE
#include "dbfile/internal/page.hxx"
#include "general/sizes.hxx"
#include <bit>
#include <iostream>
#else
module;
#include "general/sizes.hxx"
export module tinydb.dbfile.internal.page;
import std;
#include "dbfile/internal/page.hxx"
#endif // !ENABLE_MODULE

TINYDB_EXPORT
namespace tinydb::dbfile::internal {

void write_to_impl(const FreePageMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.m_pg_num * SIZEOF_PAGE);
    t_out.rdbuf()->sputc(static_cast<pt_num>(PageType::Free));
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&t_meta.m_next_pg),
                         sizeof(t_meta.m_next_pg));
}

void read_from_impl(FreePageMeta& t_meta, std::istream& t_in) {
    t_in.seekg(t_meta.m_pg_num * SIZEOF_PAGE);
    auto pagetype = static_cast<uint8_t>(t_in.rdbuf()->sbumpc());
    if (pagetype != static_cast<pt_num>(PageType::Free)) {
        // I should return something more meaningful here.
        return;
    }
    t_in.rdbuf()->sgetn(std::bit_cast<char*>(&t_meta.m_next_pg),
                        sizeof(t_meta.m_next_pg));
}

void write_to_impl(const BTreeLeafMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.m_pg_num * SIZEOF_PAGE);
    t_out.rdbuf()->sputc(static_cast<pt_num>(PageType::BTreeLeaf));
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&t_meta.m_n_rows),
                         sizeof(t_meta.m_n_rows));
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&t_meta.m_first_free),
                         sizeof(t_meta.m_first_free));
}

void read_from_impl(BTreeLeafMeta& t_meta, std::istream& t_in) {
    t_in.seekg(t_meta.m_pg_num * SIZEOF_PAGE);
    auto pagetype = static_cast<uint8_t>(t_in.rdbuf()->sbumpc());
    if (pagetype != static_cast<pt_num>(PageType::BTreeLeaf)) {
        // I should return something more meaningful here.
        return;
    }
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&t_meta.m_n_rows),
                        sizeof(t_meta.m_n_rows));
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&t_meta.m_first_free),
                        sizeof(t_meta.m_first_free));
}

void write_to_impl(const BTreeInternalMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.m_pg_num * SIZEOF_PAGE);
    t_out.rdbuf()->sputc(static_cast<pt_num>(PageType::BTreeInternal));
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&t_meta.m_n_keys),
                         sizeof(t_meta.m_n_keys));
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&t_meta.m_first_free),
                         sizeof(t_meta.m_first_free));
}

void read_from_impl(BTreeInternalMeta& t_meta, std::istream& t_in) {
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

void write_to_impl(const HeapMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.m_pg_num * SIZEOF_PAGE);
    t_out.rdbuf()->sputc(static_cast<pt_num>(PageType::Heap));
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&t_meta.m_next_pg),
                         sizeof(t_meta.m_next_pg));
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&t_meta.m_first_free),
                         sizeof(t_meta.m_first_free));
}

void read_from_impl(HeapMeta& t_meta, std::istream& t_in) {
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
