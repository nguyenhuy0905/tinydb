module;
#include "general/offsets.hxx"
#ifndef IMPORT_STD
#include <bit>
#include <iostream>
#endif
export module tinydb.dbfile.internal.freelist;
import tinydb.dbfile.internal.page;
#ifdef IMPORT_STD
import std;
#endif

namespace tinydb::dbfile::internal {

/**
 * @class FreeList
 * @brief Contains data about the freelist of a database file.
 * @details The streams passed into any function of this class are expected to
 * be the same, or at the very least, point to the same file/string.
 *
 * This is be guaranteed if the FreeList is only used in context of a database
 * file.
 *
 */
export class FreeList {
  public:
    // This should be written into the header of the database file.
    // This is constant-sized so I expect this to be written before the table
    // definition.

    /**
     * @brief Reads the content of the freelist from the specified database
     * file.
     *
     * @param t_in The specified database file.
     */
    static auto construct_from(std::istream& t_in) -> FreeList {
        uint32_t first_free{0};
        t_in.seekg(FREELIST_PTR_OFF);
        t_in.rdbuf()->sgetn(std::bit_cast<char*>(&first_free),
                            sizeof(first_free));
        return FreeList{first_free};
    }

    /**
     * @brief Default-initializes a FreeListMeta, and writes its content to the
     * database file.
     *
     * @param t_in The specified database file.
     */
    static auto default_init(uint32_t t_first_free_pg, std::ostream& t_out)
        -> FreeList {
        auto ret = FreeList{t_first_free_pg};
        t_out.seekp(DBFILE_SIZE_OFF);
        // page number starts at 0, but database file size starts at 1.
        auto filesize = t_first_free_pg + 1;
        t_out.rdbuf()->sputn(std::bit_cast<const char*>(&filesize),
                             sizeof(filesize));
        t_out.seekp(SIZEOF_PAGE * t_first_free_pg);
        FreePageMeta fpage{t_first_free_pg};
        write_to(fpage, t_out);
        t_out.seekp(FREELIST_PTR_OFF);
        t_out.rdbuf()->sputn(std::bit_cast<const char*>(&t_first_free_pg),
                             sizeof(t_first_free_pg));

        return ret;
    }

    void do_write_to(std::ostream& t_out) {
        t_out.seekp(FREELIST_PTR_OFF);
        t_out.rdbuf()->sputn(std::bit_cast<const char*>(&m_first_free_pg),
                             sizeof(m_first_free_pg));
    }

    /**
     * @brief Allocates a page of the specified page type, and formats the page
     * accordingly.
     * @tparam T The type of page to be allocated.
     * @tparam Args types of the arguments. Should be deduced by the compiler.
     * @param t_io The stream to read from and write to.
     * @param t_args The parameters, except the page number, required to
     * construct the page.
     * @return The constructed page.
     * @details Use this when you don't need the type-erasure from PageMeta.
     *   However, the I/O is way more expensive than some dynamic casting
     *   anyways.
     *
     *   If you want a PageMeta instead, simply wrap the return value of this
     * function inside a PageMeta constructor.
     *
     */
    template <typename T, typename... Args>
        requires std::is_base_of_v<PageMixin, T>
    auto allocate_page(std::iostream& t_io, Args... t_args) -> T {
        auto page = T{next_free_page(t_io), t_args...};
        write_to(page, t_io);
        return page;
    }

    /**
     * @brief Deallocates the page in the database pointed to by the PageMeta.
     * @details There is no safety check done. One can deallocate the header
     * page, which essentially destroys the entire database.
     *
     * @param t_meta The specified page. Only PageMixin is needed, since I
     * really only care about the page number.
     */
    void deallocate_page(std::iostream& t_io, PageMixin&& t_meta) {
        // move is to shut the compiler up.
        auto pgnum = std::move(t_meta).get_pg_num();
        auto curr_free_pg = m_first_free_pg;
        // this should never happen anyways.
        if (curr_free_pg == pgnum) {
            return;
        }
        if (curr_free_pg > pgnum) {
            t_io.seekp(pgnum * SIZEOF_PAGE);
            write_to(FreePageMeta{pgnum, curr_free_pg}, t_io);
            m_first_free_pg = pgnum;
            return;
        }
        uint32_t prev_free_pg{0};
        while (curr_free_pg < pgnum) {
            prev_free_pg = curr_free_pg;
            // not best performance-wise. May need to improve later.
            // FreePageMeta freepg{curr_free_pg};
            // read_from(freepg, t_io);
            auto freepg = read_from<FreePageMeta>(curr_free_pg, t_io);
            curr_free_pg = freepg.get_pg_num();
        }
        // the same as inserting a node to a linked list.
        write_to(FreePageMeta{prev_free_pg, pgnum}, t_io);
        write_to(FreePageMeta{pgnum, curr_free_pg}, t_io);
    }

  private:
    FreeList(uint32_t t_first_free_pg) : m_first_free_pg{t_first_free_pg} {}
    // first free page.
    uint32_t m_first_free_pg;

    /**
     * @brief Returns the current `m_first_free_pg`, reads the stream passed in,
     * and updates `m_first_free_pg` to be the next free page.
     * @return The current `m_first_free_pg`.
     */
    [[nodiscard]] auto next_free_page(std::iostream& t_io) -> page_ptr_t {
        auto old_first_free = m_first_free_pg;
        // FreePageMeta fpage{m_first_free_pg};
        // read_from(fpage, t_io);
        auto fpage = read_from<FreePageMeta>(old_first_free, t_io);
        auto new_first_free = fpage.get_next_pg();

        if (new_first_free != 0) {
            m_first_free_pg = new_first_free;
            return old_first_free;
        }

        // need to grab a new page.
        t_io.seekg(DBFILE_SIZE_OFF);
        uint32_t filesize{0};
        t_io.rdbuf()->sgetn(std::bit_cast<char*>(&filesize), sizeof(filesize));
        filesize++;
        t_io.seekp(DBFILE_SIZE_OFF);
        t_io.rdbuf()->sputn(std::bit_cast<const char*>(&filesize),
                            sizeof(filesize));
        FreePageMeta newfree{filesize - 1, 0};
        write_to(newfree, t_io);
        fpage.update_next_pg(filesize - 1);
        write_to(fpage, t_io);
        m_first_free_pg = filesize - 1;
        do_write_to(t_io);

        return old_first_free;
    }
};

} // namespace tinydb::dbfile::internal
