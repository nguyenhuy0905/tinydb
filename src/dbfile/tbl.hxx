#ifndef TINYDB_DBFILE_TBL_HXX
#define TINYDB_DBFILE_TBL_HXX

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace tinydb::dbfile {

using ColID = uint8_t;
using EntrySiz = uint16_t;

/**
 * @class ColumnMeta
 * @brief Metadata of one column
 *
 */
struct ColumnMeta {
    std::string m_name;
    EntrySiz m_size;
    ColID m_col_id;
    EntrySiz m_offset;
};

/**
 * @class Table
 * @brief The table meta contains a bunch of column metas.
 *
 */
class TableMeta {
  public:
    TableMeta() = default;
    static auto read_from(std::filesystem::path t_path) -> TableMeta;
    auto add_column(ColumnMeta&& t_colmeta);
    auto remove_column(ColID t_id);
    /**
     * @brief Overwrite the content of a file with the table metadata.
     *
     * @param t_path The path to write to.
     */
    auto write_to(std::filesystem::path t_path);

  private:
    std::unordered_map<ColID, ColumnMeta> m_entries;
    std::string m_name;
};

} // namespace tinydb::dbfile

#endif // !TINYDB_DBFILE_TBL_HXX
