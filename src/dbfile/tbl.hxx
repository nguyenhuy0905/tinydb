#ifndef TINYDB_DBFILE_TBL_HXX
#define TINYDB_DBFILE_TBL_HXX

#include "modules.hxx"
#ifndef ENABLE_MODULE
#include <cstdint>
#include <iosfwd>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <functional>
#endif // !ENABLE_MODULE

TINYDB_EXPORT 
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
    static auto read_from(std::istream& t_in) -> TableMeta;
    /**
     * @brief I changed this to noexcept since I think the lookup function
     * should not throw an exception.
     * @return The column meta with the specified ID, or nullopt if there isn't
     * one
     */
    template <typename T>
        requires std::convertible_to<T, std::string>
    auto get_column(T&& t_name) noexcept
        -> std::optional<rw<const ColumnMeta>> const {
        try {
            return {{m_entries.at(std::forward<T>(t_name))}};
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
        return m_entries.emplace(t_colmeta.m_name, std::forward<T>(t_colmeta))
            .second;
    }

    /**
     * @brief Sets a specific column as the key.
     * If one's writing this into a database file, this must be set before
     * calling write_to.
     *
     * @param t_key The name of the column that should be the key.
     * @return false if the specified key doesn't exist in the entry map.
     *   true otherwise.
     */
    template <typename T>
        requires std::convertible_to<T, std::string>
    auto set_key(T&& t_key) -> bool {
        if (m_entries.find(t_key) == m_entries.end()) {
            return false;
        }

        m_key = std::forward<T>(t_key);
        return true;
    }
    /**
     * @brief Note; this is marked as noexcept because I believe the default
     * hash and equality function for size_t don't throw anything.
     *
     * @param t_id
     * @return
     */
    template <typename T>
        requires std::convertible_to<T, std::string>
    auto remove_column(T&& t_name) noexcept -> bool {
        return m_entries.erase(std::forward<T>(t_name)) == 1;
    }
    /**
     * @brief Overwrite the content of a file with the table metadata.
     *
     * @param t_path The path to write to.
     */
    void write_to(std::ostream& t_out);

  private:
    std::unordered_map<std::string, ColumnMeta> m_entries;
    std::string m_name;
    std::string m_key;
    static constexpr uint16_t TABLE_OFFSET = 18;
};

} // namespace tinydb::dbfile

#endif // !TINYDB_DBFILE_TBL_HXX
