#ifndef ENABLE_MODULE
#include "dbfile/internal/page_serialize.hxx"
#include "dbfile/internal/page_base.hxx"
#include "dbfile/internal/page_meta.hxx"
#include "general/sizes.hxx"
#include <bit>
#include <cassert>
#include <iostream>
#else
module;
#include <cassert>
#include <cstdint>
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
    t_out.rdbuf()->sputc(static_cast<pt_num_t>(PageType::Free));

    auto next_pg = t_meta.get_next_pg();
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&next_pg), sizeof(next_pg));
}

template <>
auto read_from<FreePageMeta>(page_ptr_t t_pg_num, std::istream& t_in)
    -> FreePageMeta {
    t_in.seekg(t_pg_num * SIZEOF_PAGE);
    [[maybe_unused]]
    auto pagetype = t_in.rdbuf()->sbumpc();
    assert(pagetype == static_cast<pt_num_t>(PageType::Free));
    // TODO: think of a new checking mechanism

    // auto pagetype = static_cast<uint8_t>(t_in.rdbuf()->sbumpc());
    // if (pagetype != static_cast<pt_num>(PageType::Free)) {
    //     // I should return something more meaningful here.
    //     return;
    // }

    uint32_t next_pg{0};
    t_in.rdbuf()->sgetn(std::bit_cast<char*>(&next_pg), sizeof(next_pg));
    return {t_pg_num, next_pg};
}

void write_to(const BTreeLeafMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.get_pg_num() * SIZEOF_PAGE);
    t_out.rdbuf()->sputc(static_cast<pt_num_t>(PageType::BTreeLeaf));
    auto nrows = t_meta.get_n_rows();
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&nrows), sizeof(nrows));
    auto first_free = t_meta.get_first_free();
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&first_free),
                         sizeof(first_free));
}

template <>
auto read_from<BTreeLeafMeta>(page_ptr_t t_pg_num, std::istream& t_in)
    -> BTreeLeafMeta {
    t_in.seekg(t_pg_num * SIZEOF_PAGE);

    [[maybe_unused]]
    auto pagetype = t_in.rdbuf()->sbumpc();
    assert(pagetype == static_cast<pt_num_t>(PageType::BTreeLeaf));
    // if (pagetype != static_cast<pt_num>(PageType::BTreeLeaf)) {
    //     // I should return something more meaningful here.
    //     return;
    // }

    uint16_t nrows{0};
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&nrows), sizeof(nrows));
    uint16_t first_free{0};
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&first_free), sizeof(first_free));
    return {t_pg_num, nrows, first_free};
}

void write_to(const BTreeInternalMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.get_pg_num() * SIZEOF_PAGE);
    t_out.rdbuf()->sputc(static_cast<pt_num_t>(PageType::BTreeInternal));
    auto nkeys = t_meta.get_n_keys();
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&nkeys), sizeof(nkeys));
    auto first_free = t_meta.get_first_free_off();
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&first_free),
                         sizeof(first_free));
}

template <>
auto read_from<BTreeInternalMeta>(page_ptr_t t_pg_num, std::istream& t_in)
    -> BTreeInternalMeta {
    t_in.seekg(t_pg_num * SIZEOF_PAGE);
    [[maybe_unused]]
    auto pagetype = t_in.rdbuf()->sbumpc();
    assert(pagetype == static_cast<pt_num_t>(PageType::BTreeInternal));
    // auto pagetype = static_cast<uint8_t>(t_in.rdbuf()->sbumpc());
    // if (pagetype != static_cast<pt_num>(PageType::BTreeInternal)) {
    //     // I should return something more meaningful here.
    //     return;
    // }
    uint16_t nkeys{0};
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&nkeys), sizeof(nkeys));
    uint16_t first_free{0};
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&first_free), sizeof(first_free));
    return {t_pg_num, nkeys, first_free};
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
    t_out.seekp(t_meta.get_pg_num() * SIZEOF_PAGE);
    auto& rdbuf = *t_out.rdbuf();
    rdbuf.sputc(static_cast<char>(PageType::Heap));
    auto nextpg = t_meta.get_next_pg();
    rdbuf.sputn(std::bit_cast<const char*>(&nextpg), sizeof(nextpg));
    auto first_free = t_meta.get_first_free_off();
    rdbuf.sputn(std::bit_cast<const char*>(&first_free), sizeof(first_free));
    auto min_pair = t_meta.get_min_pair();
    rdbuf.sputn(std::bit_cast<const char*>(&min_pair.first),
                sizeof(min_pair.first));
    rdbuf.sputn(std::bit_cast<const char*>(&min_pair.second),
                sizeof(min_pair.second));
    auto max_pair = t_meta.get_max_pair();
    rdbuf.sputn(std::bit_cast<const char*>(&max_pair.first),
                sizeof(max_pair.first));
    rdbuf.sputn(std::bit_cast<const char*>(&max_pair.second),
                sizeof(max_pair.second));
}

template <>
auto read_from<HeapMeta>(page_ptr_t t_pg_num, std::istream& t_in) -> HeapMeta {
    t_in.seekg(t_pg_num * SIZEOF_PAGE);
    auto& rdbuf = *t_in.rdbuf();
    // rdbuf.sbumpc();

    [[maybe_unused]]
    auto pagetype = rdbuf.sbumpc();
    assert(pagetype == static_cast<pt_num_t>(PageType::Heap));
    // if (pagetype != static_cast<pt_num_t>(PageType::Heap)) {
    //     // I should return something more meaningful here.
    //     return;
    // }

    page_ptr_t nextpg{0};
    rdbuf.sgetn(std::bit_cast<char*>(&nextpg), sizeof(nextpg));
    page_off_t first_free{0};
    rdbuf.sgetn(std::bit_cast<char*>(&first_free), sizeof(first_free));
    std::pair<page_off_t, page_off_t> min_pair{0, 0};
    rdbuf.sgetn(std::bit_cast<char*>(&min_pair.first), sizeof(min_pair.first));
    rdbuf.sgetn(std::bit_cast<char*>(&min_pair.second),
                sizeof(min_pair.second));

    std::pair<page_off_t, page_off_t> max_pair{0, 0};
    rdbuf.sgetn(std::bit_cast<char*>(&max_pair.first), sizeof(max_pair.first));
    rdbuf.sgetn(std::bit_cast<char*>(&max_pair.second),
                sizeof(max_pair.second));
    return {t_pg_num, nextpg, first_free, min_pair, max_pair};
}

static_assert(PageSerializable<FreePageMeta>);
static_assert(PageSerializable<BTreeLeafMeta>);
static_assert(PageSerializable<BTreeInternalMeta>);
static_assert(PageSerializable<HeapMeta>);

} // namespace tinydb::dbfile::internal
