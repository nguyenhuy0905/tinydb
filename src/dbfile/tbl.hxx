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

enum class NumericColType : uint8_t {
    Number, // 16-bit   
};

struct VarCharColType {
    EntrySiz m_len;
};

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
    template <typename T>
        requires std::convertible_to<T, std::string>
    explicit TableMeta(T&& t_name) : m_name(std::forward<T>(t_name)) {}
    TableMeta(TableMeta&& t_meta) = default;
    TableMeta(const TableMeta& t_meta) = default;
    auto operator=(const TableMeta& t_meta) -> TableMeta& = default;
    auto operator=(TableMeta&& t_meta) -> TableMeta& = default;
    ~TableMeta() = default;
    /**
     * @brief Reads the table metadata from a database file.
     *
     * @param t_path The path to that file.
     * @return
     * - The table metadata if the read and data extraction is successful.
     * - nullopt otherwise.
     * - This API will be changed later. optional is a terrible but
     * quick-and-seemingly-easy way to say there's something wrong.
     */
    static auto
    read_from(const std::filesystem::path& t_path) -> std::optional<TableMeta>;
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
        requires std::convertible_to<T, ColumnMeta>
    auto add_column(T&& t_colmeta) -> bool {
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
