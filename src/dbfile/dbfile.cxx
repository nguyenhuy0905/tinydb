#ifndef ENABLE_MODULE
#include "version.hxx"
#include "freelist.hxx"
#include "tbl.hxx"
#include <bit>
#include <iostream>
#else
module;
#include "version.hxx"
#include <cstdint>
export module tinydb.dbfile;
import tinydb.dbfile.page;
import tinydb.dbfile.freelist;
import tinydb.dbfile.tbl;
import std;
#include "dbfile.hxx"
#endif // !ENABLE_MODULE


namespace tinydb::dbfile {

DbFile::DbFile(FreeListMeta&& t_fl, TableMeta&& t_tbl,
               std::unique_ptr<std::iostream>&& t_io)
    : m_tbl{std::move(t_tbl)}, m_rw{std::move(t_io)},
      m_freelist{std::move(t_fl)} {}

void DbFile::write_init() {
    m_rw->seekp(0);
    // version.
    m_rw->rdbuf()->sputn(std::bit_cast<const char*>(&tinydb::VERSION_MAJOR),
                         sizeof(tinydb::VERSION_MAJOR));
    m_rw->rdbuf()->sputn(std::bit_cast<const char*>(&tinydb::VERSION_MINOR),
                         sizeof(tinydb::VERSION_MINOR));
    m_rw->rdbuf()->sputn(std::bit_cast<const char*>(&tinydb::VERSION_PATCH),
                         sizeof(tinydb::VERSION_PATCH));
    // dbfile size. The first page is all metadata.
    constexpr uint32_t initial_size = 1;
    m_rw->rdbuf()->sputn(std::bit_cast<const char*>(&initial_size),
                         sizeof(initial_size));
    m_freelist.write_to(*m_rw);
    m_tbl.write_to(*m_rw);
}

auto DbFile::construct_from(std::unique_ptr<std::iostream>&& t_io) -> DbFile {
    auto freelist = FreeListMeta::construct_from(*t_io);
    auto tbl = TableMeta::read_from(*t_io);
    return DbFile{std::move(freelist), std::move(tbl), std::move(t_io)};
}

} // namespace tinydb::dbfile
