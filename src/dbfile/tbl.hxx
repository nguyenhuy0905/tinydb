#ifndef TINYDB_DBFILE_TBL_HXX
#define TINYDB_DBFILE_TBL_HXX

#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>

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
    static_assert(!std::is_trivially_copyable_v<std::filesystem::path>);
    static_assert(!std::is_trivially_copyable_v<ColumnMeta>);

  public:
    template <typename T> using rw = std::reference_wrapper<T>;
    TableMeta() = default;
    static auto read_from(const std::filesystem::path& t_path) -> TableMeta;
    /**
     * @brief I changed this to noexcept since I think the lookup function
     * should not throw an exception.
     * @return The column meta with the specified ID, or nullopt if there isn't
     * one
     */
    auto get_column(ColID t_id) noexcept
        -> std::optional<rw<const ColumnMeta>> const {
        try {
            return {{m_entries.at(t_id)}};
        } catch (std::out_of_range e) {
            return {std::nullopt};
        }
    }
    /**
     * @brief Inserts a column into the table.
     *
     * @tparam T ColumnMeta or references (lvalue or rvalue) to ColumnMeta.
     * @param t_colmeta
     * @return Whether the column meta was successfully added.
     */
    template <typename T>
        requires std::same_as<std::remove_cvref<T>, ColumnMeta>
    auto add_column(const ColumnMeta& t_colmeta) -> bool {
        return m_entries.emplace(t_colmeta.m_col_id, std::forward<T>(t_colmeta))
            .second;
    }
    /**
     * @brief Note; this is marked as noexcept because I believe the default
     * hash and equality function for size_t don't throw anything.
     *
     * @param t_id
     * @return
     */
    auto remove_column(ColID t_id) noexcept -> bool {
        return m_entries.erase(t_id) == 1;
    }
    /**
     * @brief Overwrite the content of a file with the table metadata.
     *
     * @param t_path The path to write to.
     */
    void write_to(const std::filesystem::path& t_path);

  private:
    std::map<ColID, ColumnMeta> m_entries;
    std::string m_name;
};

} // namespace tinydb::dbfile

#endif // !TINYDB_DBFILE_TBL_HXX
