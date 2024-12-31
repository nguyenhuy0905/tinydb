#ifndef TINYDB_DBFILE_DBFILE_HXX
#define TINYDB_DBFILE_DBFILE_HXX

#include <fstream>

namespace tinydb::dbfile {

/**
 * @class DbFile
 * @brief The database file itself.
 *
 */
class DbFileMeta {
  public:
  private:
    std::fstream m_dbfile;
    // TODO: finish decl dbfile.
};

} // namespace tinydb::dbfile

#endif // !TINYDB_DBFILE_DBFILE_HXX
