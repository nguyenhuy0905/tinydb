#ifndef TINYDB_DBFILE_ROW_BUILER_HXX
#define TINYDB_DBFILE_ROW_BUILER_HXX

#include "general/modules.hxx"
#ifndef ENABLE_MODULES
#include "dbfile/coltype.hxx"
#include "dbfile/dbfile.hxx"
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#endif // !ENABLE_MODULES

TINYDB_EXPORT
namespace tinydb::dbfile {

/**
 * @class RowBuilder
 * @brief It builds a data row. Then it can be used for writing data into a
 * database file.
 *
 */
class RowBuilder {
public:
  explicit RowBuilder(DbFile&);
  auto add_column_data(std::string_view, column::InMemCol);
  auto remove_column(std::string_view);
  auto get_column(std::string_view t_sv);

private:
  /**
   * @class TransparentStringHash
   * @brief A string hasher that allows comparisons of, say, `std::string` to
   * `std::string_view`. The official term is "heterogeneous lookup" I think?
   * Shamelessly taken from cppreference.
   */
  struct TransparentStringHash {
    using hash_type = std::hash<std::string_view>;
    // this can be absolutely anything.
    using is_transparent = void;

    [[nodiscard]] constexpr auto operator()(const char* t_str) const noexcept
        -> size_t {
      return std::hash<std::string_view>{}(t_str);
    }
    [[nodiscard]] constexpr auto
    operator()(std::string_view t_str) const noexcept -> size_t {
      return std::hash<std::string_view>{}(t_str);
    }
    [[nodiscard]] auto operator()(const std::string& t_str) const noexcept
        -> size_t {
      return std::hash<std::string>{}(t_str);
    }
  };

  std::unordered_map<std::string, column::InMemCol, TransparentStringHash,
                     std::equal_to<>>
      m_rowdata;
  std::reference_wrapper<DbFile> m_db;
};

} // namespace tinydb::dbfile

#endif // !TINYDB_DBFILE_ROW_BUILER_HXX
