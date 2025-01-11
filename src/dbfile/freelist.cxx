#include "freelist.hxx"
#include "page.hxx"
#include "sizes.hxx"
#include "offsets.hxx"
#include <bit>
#include <iostream>

namespace tinydb::dbfile {

auto FreeListMeta::default_init(uint32_t t_first_free_pg,
                                std::ostream& t_out) -> FreeListMeta {
    auto ret = FreeListMeta{t_first_free_pg};
    t_out.seekp(FREELIST_OFF);
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&t_first_free_pg),
                         sizeof(t_first_free_pg));
    t_out.seekp(SIZEOF_PAGE * t_first_free_pg);
    auto fpage = FreePageMeta{t_first_free_pg};
    write_to_impl(fpage, t_out);

    return ret;
}

auto FreeListMeta::construct_from(std::istream& t_in) -> FreeListMeta {
    uint32_t first_free{0};
    t_in.seekg(FREELIST_OFF);
    t_in.rdbuf()->sgetn(std::bit_cast<char*>(&first_free), sizeof(first_free));
    return FreeListMeta{first_free};
}

auto FreeListMeta::next_free_page(std::iostream& t_io) -> uint32_t {
    auto old_first_free = m_first_free_pg;
    FreePageMeta fpage{m_first_free_pg};
    read_from_impl(fpage, t_io);
    auto new_first_free = fpage.get_next_pg();

    if (new_first_free != 0) {
        return old_first_free;
    }

    // need to grab a new page.
    t_io.seekg(DBFILE_SIZE_OFF);
    uint32_t filesize{0};
    t_io.rdbuf()->sgetn(std::bit_cast<char*>(&filesize), sizeof(filesize));
    filesize++;
    t_io.seekp(DBFILE_SIZE_OFF);
    t_io.rdbuf()->sputn(std::bit_cast<const char*>(&filesize), sizeof(filesize));
    FreePageMeta newfree{filesize - 1, 0};
    write_to_impl(newfree, t_io);

    m_first_free_pg = filesize - 1;

    return old_first_free;
}

} // namespace tinydb::dbfile
