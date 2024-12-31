#ifndef TINYDB_DBFILE_FREELIST_HXX
#define TINYDB_DBFILE_FREELIST_HXX

#include <cstddef>
#include <cstdint>
#include <iosfwd>
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
     * @brief Reads the content of the freelist from the specified database file.
     *
     * @param path The specified database file.
     */
    static auto read_from(std::istream& t_in) -> FreeListMeta;

    /**
     * @brief Writes the content of this freelist into the specified database file.
     *
     * @param t_out The specified database file.
     */
    void write_to(std::ostream& t_out);

    /**
     * @return The page number of the first free page available.
     *
     * This only guarantees one free page.
     */
    auto get_first_free_page() -> uint16_t { return m_first_free_ptr; }

  private:
    uint16_t m_first_free_ptr;
    // we may need more down here. Who knows?
};

} // namespace tinydb::dbfile

#endif // !TINYDB_DBFILE_FREELIST_HXX
