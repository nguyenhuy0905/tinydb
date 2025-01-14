#ifndef TINYDB_DBFILE_DBFILE_HXX
#define TINYDB_DBFILE_DBFILE_HXX

#include "modules.hxx"
#ifndef ENABLE_MODULE
#include "freelist.hxx"
#include "tbl.hxx"
#include <iosfwd>
#include <memory>
#endif

TINYDB_EXPORT
namespace tinydb::dbfile {

/**
 * @class DbFile
 * @brief The database file itself.
 *
 */
class DbFile {
  public:
    static auto construct_from(std::unique_ptr<std::iostream> t_io) -> DbFile;

    static auto new_empty(std::unique_ptr<std::iostream>&& t_io) -> DbFile;

    void write_init();

    void add_column(std::string&& t_name);

  private:
    DbFile(FreeListMeta&& t_fl, TableMeta&& t_tbl,
           std::unique_ptr<std::iostream>&& t_io);
    TableMeta m_tbl;
    std::unique_ptr<std::iostream> m_rw;
    FreeListMeta m_freelist;
    // for now there's no need for heap yet.
};

} // namespace tinydb::dbfile

#endif // !TINYDB_DBFILE_DBFILE_HXX
