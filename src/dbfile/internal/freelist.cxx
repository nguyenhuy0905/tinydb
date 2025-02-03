/**
 * @file freelist.cxx
 * @brief Defines the interfaces declared in freelist.hxx
 */

#ifdef ENABLE_MODULES
module;
#include "general/modules.hxx"
#include "general/offsets.hxx"
#ifndef IMPORT_STD
#include <bit>
#include <iostream>
#endif
export module tinydb.dbfile.internal.freelist;
import tinydb.dbfile.internal.page;
#ifdef IMPORT_STD
import std;
#endif
#else
#include "dbfile/internal/page_base.hxx"
#include "dbfile/internal/page_meta.hxx"
#include "dbfile/internal/page_serialize.hxx"
#include "general/offsets.hxx"
#include <bit>
#include <iostream>
#endif // ENABLE_MODULES

#include "dbfile/internal/freelist.hxx"

namespace tinydb::dbfile::internal {

auto FreeList::construct_from(std::istream& t_in) -> FreeList {
  uint32_t first_free{0};
  t_in.seekg(FREELIST_PTR_OFF);
  t_in.rdbuf()->sgetn(std::bit_cast<char*>(&first_free), sizeof(first_free));
  return FreeList{first_free};
}

auto FreeList::default_init(uint32_t t_first_free_pg, std::ostream& t_out)
    -> FreeList {
  auto ret = FreeList{t_first_free_pg};
  t_out.seekp(DBFILE_SIZE_OFF);
  // page number starts at 0, but database file size starts at 1.
  auto filesize = t_first_free_pg + 1;
  t_out.rdbuf()->sputn(std::bit_cast<const char*>(&filesize), sizeof(filesize));
  t_out.seekp(SIZEOF_PAGE * t_first_free_pg);
  FreePageMeta fpage{t_first_free_pg};
  write_to(fpage, t_out);
  t_out.seekp(FREELIST_PTR_OFF);
  t_out.rdbuf()->sputn(std::bit_cast<const char*>(&t_first_free_pg),
                       sizeof(t_first_free_pg));

  return ret;
}

void FreeList::do_write_to(std::ostream& t_out) {
  t_out.seekp(FREELIST_PTR_OFF);
  t_out.rdbuf()->sputn(std::bit_cast<const char*>(&m_first_free_pg),
                       sizeof(m_first_free_pg));
}

void FreeList::deallocate_page(std::iostream& t_io, PageMixin&& t_meta) {
  // move is to shut the compiler up.
  auto pgnum = std::move(t_meta).get_pg_num();
  auto curr_free_pg = m_first_free_pg;
  // this should never happen anyways.
  if (curr_free_pg == pgnum) {
    return;
  }
  if (curr_free_pg > pgnum) {
    t_io.seekp(pgnum * SIZEOF_PAGE);
    write_to(FreePageMeta{pgnum, curr_free_pg}, t_io);
    m_first_free_pg = pgnum;
    return;
  }
  uint32_t prev_free_pg{0};
  while (curr_free_pg < pgnum) {
    prev_free_pg = curr_free_pg;
    // not best performance-wise. May need to improve later.
    // FreePageMeta freepg{curr_free_pg};
    // read_from(freepg, t_io);
    auto freepg = read_from<FreePageMeta>(curr_free_pg, t_io);
    curr_free_pg = freepg.get_pg_num();
  }
  // the same as inserting a node to a linked list.
  write_to(FreePageMeta{prev_free_pg, pgnum}, t_io);
  write_to(FreePageMeta{pgnum, curr_free_pg}, t_io);
}

[[nodiscard]] auto FreeList::next_free_page(std::iostream& t_io) -> page_ptr_t {
  auto old_first_free = m_first_free_pg;
  // FreePageMeta fpage{m_first_free_pg};
  // read_from(fpage, t_io);
  auto fpage = read_from<FreePageMeta>(old_first_free, t_io);
  auto new_first_free = fpage.get_next_pg();

  if (new_first_free != 0) {
    m_first_free_pg = new_first_free;
    return old_first_free;
  }

  // need to grab a new page.
  t_io.seekg(DBFILE_SIZE_OFF);
  uint32_t filesize{0};
  t_io.rdbuf()->sgetn(std::bit_cast<char*>(&filesize), sizeof(filesize));
  filesize++;
  t_io.seekp(DBFILE_SIZE_OFF);
  t_io.rdbuf()->sputn(std::bit_cast<const char*>(&filesize), sizeof(filesize));
  FreePageMeta newfree{filesize - 1, 0};
  write_to(newfree, t_io);
  fpage.update_next_pg(filesize - 1);
  write_to(fpage, t_io);
  m_first_free_pg = filesize - 1;
  do_write_to(t_io);

  return old_first_free;
}

} // namespace tinydb::dbfile::internal
