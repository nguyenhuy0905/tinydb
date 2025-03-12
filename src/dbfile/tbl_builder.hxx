#ifndef TINYDB_DBFILE_TBL_BUILDER_HXX
#define TINYDB_DBFILE_TBL_BUILDER_HXX

#include "tinydb_export.h"
#ifndef ENABLE_MODULES
#include "dbfile/internal/tbl.hxx"
#include <type_traits>
#endif // !ENABLE_MODULES

#ifdef ENABLE_MODULES
export namespace tinydb::dbfile {
#else
namespace tinydb::dbfile {
#endif // ENABLE_MODULES

class TINYDB_EXPORT TblBuilder {
public:
  template <typename S>
    requires std::is_convertible_v<S, std::string>
  TblBuilder(S&& t_tbl_name) : m_tbl(std::forward<S>(t_tbl_name)) {}

  auto add_column(internal::ColumnMeta&& t_col_name) -> bool;
  auto set_key(std::string_view t_key) -> bool;

private:
  internal::TableMeta m_tbl;
};
}

#endif // !TINYDB_DBFILE_TBL_BUILDER_HXX
