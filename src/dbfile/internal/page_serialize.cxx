#ifndef ENABLE_MODULE
#include "dbfile/internal/page_serialize.hxx"
#include "dbfile/internal/page_meta.hxx"
#include "general/sizes.hxx"
#include <bit>
#include <iostream>
#else
module;
#include "general/sizes.hxx"
export module tinydb.dbfile.internal.page:serialize;
export import :base;
export import :meta;
import std;
#include "dbfile/internal/page_serialize.hxx"
#endif // !ENABLE_MODULE

TINYDB_EXPORT
namespace tinydb::dbfile::internal {

void write_to(const FreePageMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.get_pg_num() * SIZEOF_PAGE);
    t_out.rdbuf()->sputc(static_cast<pt_num>(PageType::Free));

    auto next_pg = t_meta.get_next_pg();
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&next_pg), sizeof(next_pg));
}

template <>
auto read_from<FreePageMeta>(uint32_t t_pg_num, std::istream& t_in)
    -> FreePageMeta {
    t_in.seekg(t_pg_num * SIZEOF_PAGE);
    t_in.rdbuf()->sbumpc();
    // TODO: think of a new checking mechanism

    // auto pagetype = static_cast<uint8_t>(t_in.rdbuf()->sbumpc());
    // if (pagetype != static_cast<pt_num>(PageType::Free)) {
    //     // I should return something more meaningful here.
    //     return;
    // }

    uint32_t next_pg{0};
    t_in.rdbuf()->sgetn(std::bit_cast<char*>(&next_pg), sizeof(next_pg));
    return FreePageMeta{t_pg_num, next_pg};
}

void write_to(const BTreeLeafMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.get_pg_num() * SIZEOF_PAGE);
    t_out.rdbuf()->sputc(static_cast<pt_num>(PageType::BTreeLeaf));
    auto nrows = t_meta.get_n_rows();
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&nrows), sizeof(nrows));
    auto first_free = t_meta.get_first_free();
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&first_free),
                         sizeof(first_free));
}

template <>
auto read_from<BTreeLeafMeta>(uint32_t t_pg_num, std::istream& t_in)
    -> BTreeLeafMeta {
    t_in.seekg(t_pg_num * SIZEOF_PAGE);

    t_in.rdbuf()->sbumpc();
    // auto pagetype = static_cast<uint8_t>(t_in.rdbuf()->sbumpc());
    // if (pagetype != static_cast<pt_num>(PageType::BTreeLeaf)) {
    //     // I should return something more meaningful here.
    //     return;
    // }

    uint16_t nrows{0};
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&nrows), sizeof(nrows));
    uint16_t first_free{0};
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&first_free), sizeof(first_free));
    return BTreeLeafMeta{t_pg_num, nrows, first_free};
}

void write_to(const BTreeInternalMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.get_pg_num() * SIZEOF_PAGE);
    t_out.rdbuf()->sputc(static_cast<pt_num>(PageType::BTreeInternal));
    auto nkeys = t_meta.get_n_keys();
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&nkeys), sizeof(nkeys));
    auto first_free = t_meta.get_first_free_off();
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&first_free),
                         sizeof(first_free));
}

template <>
auto read_from<BTreeInternalMeta>(uint32_t t_pg_num, std::istream& t_in)
    -> BTreeInternalMeta {
    t_in.seekg(t_pg_num * SIZEOF_PAGE);
    t_in.rdbuf()->sbumpc();
    // auto pagetype = static_cast<uint8_t>(t_in.rdbuf()->sbumpc());
    // if (pagetype != static_cast<pt_num>(PageType::BTreeInternal)) {
    //     // I should return something more meaningful here.
    //     return;
    // }
    uint16_t nkeys{0};
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&nkeys), sizeof(nkeys));
    uint16_t first_free{0};
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&first_free), sizeof(first_free));
    return BTreeInternalMeta{t_pg_num, nkeys, first_free};
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

} // namespace tinydb::dbfile::internal
