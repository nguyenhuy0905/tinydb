#ifndef TINYDB_DBFILE_FREELIST_HXX
#define TINYDB_DBFILE_FREELIST_HXX

#include <cstddef>
#include <cstdint>
#include <filesystem>
namespace tinydb::dbfile {

/**
 * @class FreeListMeta
 * @brief Contains metadata about the freelist of a database file.
 *
 */
class FreeListMeta {
  public:
    // TODO: implement free list
    //
    // This should be written into the header of the database file.
    // This is constant-sized so I expect this to be written before the table
    // definition.

    /**
     * @brief Initializes a free list in the specified file.
     *
     * For now, this is planned to be at the beginning of the file. This means
     * there needs to be a change to how `TableMeta` writes stuff.
     *
     * Note: With this, I probably will have to change the open mode inside
     * `TableMeta` into out, not trunc.
     *
     * @param path The path to the specified file.
     */
    static void init_free_list(const std::filesystem::path& path);

    /**
     * @brief Reads the content of the freelist from the specified database file.
     *
     * @param path The specified database file.
     */
    static auto read_from(const std::filesystem::path& path) -> FreeListMeta;

    /**
     * @return The page number of the first free page available.
     *
     * This only guarantees one free page.
     */
    auto get_first_free_page() -> uint16_t { return m_first_free_ptr; }

    /**
     * @brief Similarly to `get_first_free_page` but we are instead looking for
     * `t_n_pages` consecutive free pages.
     *
     * @param t_n_pages
     */
    auto get_first_n_free_page(size_t t_n_pages) -> uint16_t;

  private:
    uint16_t m_first_free_ptr;
    // we may need more down here. Who knows?
};

} // namespace tinydb::dbfile

#endif // !TINYDB_DBFILE_FREELIST_HXX
