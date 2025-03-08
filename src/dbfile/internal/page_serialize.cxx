/**
 * @file page_serialize.cxx
 * @brief Definitions for page_serialize.hxx.
 */

#ifdef ENABLE_MODULES
module;
#include "general/modules.hxx"
#include "general/sizes.hxx"
#include "general/utils.hxx"
#include <cassert>
#ifndef IMPORT_STD
#include <bit>
#include <cstdint>
#include <iostream>
#include <memory>
#include <print>
#include <utility>
#endif
export module tinydb.dbfile.internal.page:serialize;
import :base;
import :meta;
#ifdef IMPORT_STD
import std;
#endif
#else
#include "dbfile/internal/page_base.hxx"
#include "dbfile/internal/page_meta.hxx"
#include "general/sizes.hxx"
#include "general/utils.hxx"
#include <bit>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <print>
#include <utility>
#endif // ENABLE_MODULES

#include "dbfile/internal/page_serialize.hxx"

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

  page_ptr_t next_pg{0};
  t_in.rdbuf()->sgetn(std::bit_cast<char*>(&next_pg), sizeof(next_pg));
  return {t_pg_num, next_pg};
}

void write_to(const BTreeLeafMeta& t_meta, std::ostream& t_out) {
  t_out.seekp(t_meta.get_pg_num() * SIZEOF_PAGE);
  t_out.rdbuf()->sputc(static_cast<pt_num_t>(PageType::BTreeLeaf));
  auto nrows = t_meta.get_n_rows();
  t_out.rdbuf()->sputn(std::bit_cast<const char*>(&nrows), sizeof(nrows));
  auto first_free = t_meta.get_first_free_off();
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

void write_to(const HeapMeta& t_meta, std::ostream& t_out) {
  t_out.seekp(t_meta.get_pg_num() * SIZEOF_PAGE);
  auto& rdbuf = *t_out.rdbuf();
  rdbuf.sputc(static_cast<char>(PageType::Heap));
  auto nextpg = t_meta.get_next_pg();
  rdbuf.sputn(std::bit_cast<const char*>(&nextpg), sizeof(nextpg));
  auto prevpg = t_meta.get_prev_pg();
  rdbuf.sputn(std::bit_cast<const char*>(&prevpg), sizeof(prevpg));
  auto first_free = t_meta.get_first_free_off();
  rdbuf.sputn(std::bit_cast<const char*>(&first_free), sizeof(first_free));
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
  page_ptr_t prevpg{0};
  rdbuf.sgetn(std::bit_cast<char*>(&prevpg), sizeof(prevpg));
  page_off_t first_free{0};
  rdbuf.sgetn(std::bit_cast<char*>(&first_free), sizeof(first_free));
  auto max_pair =
      std::make_pair(static_cast<page_off_t>(0), static_cast<page_off_t>(0));
  rdbuf.sgetn(std::bit_cast<char*>(&max_pair.first), sizeof(max_pair.first));
  rdbuf.sgetn(std::bit_cast<char*>(&max_pair.second), sizeof(max_pair.second));
  return {t_pg_num, nextpg, prevpg, first_free, max_pair};
}

// technically, write_to could be a templated function, relying on template
// specialization.

static_assert(PageSerializable<FreePageMeta>);
static_assert(PageSerializable<BTreeLeafMeta>);
static_assert(PageSerializable<BTreeInternalMeta>);
static_assert(PageSerializable<HeapMeta>);

} // namespace tinydb::dbfile::internal
