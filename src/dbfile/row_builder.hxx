#ifndef TINYDB_DBFILE_ROW_BUILER_HXX
#define TINYDB_DBFILE_ROW_BUILER_HXX

#include "general/modules.hxx"
#ifndef ENABLE_MODULES
#include "dbfile/coltype.hxx"
#include "dbfile/dbfile.hxx"
#include <cstddef>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#endif // !ENABLE_MODULES

TINYDB_EXPORT
namespace tinydb::dbfile {

class RowBuilder {
public:
  explicit RowBuilder(DbFile&);
  auto add_column(std::string_view);

private:
  std::unordered_map<std::string, std::span<std::byte>> m_rowdata;
  std::reference_wrapper<DbFile> m_db;
};

} // namespace tinydb::dbfile

#endif // !TINYDB_DBFILE_ROW_BUILER_HXX
