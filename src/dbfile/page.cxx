#include "page.hxx"
#include <bit>
#include <iostream>

namespace tinydb::dbfile {

void write_to_impl(const FreePageMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.m_page_num * PAGESIZ);
    t_out.rdbuf()->sputc(static_cast<pt_num>(PageType::Free));
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&t_meta.m_next_pg),
                         sizeof(t_meta.m_next_pg));
}

void read_from_impl(FreePageMeta& t_meta, std::istream& t_in) {
    t_in.seekg(t_meta.m_page_num * PAGESIZ);
    auto pagetype = static_cast<uint8_t>(t_in.rdbuf()->sbumpc());
    if (pagetype != static_cast<pt_num>(PageType::Free)) {
        // I should return something more meaningful here.
        return;
    }
    t_in.rdbuf()->sgetn(std::bit_cast<char*>(&t_meta.m_next_pg),
                        sizeof(t_meta.m_next_pg));
}

void write_to_impl(const BTreeLeafMeta& t_meta, std::ostream& t_out) {
    t_out.seekp(t_meta.m_page_num * PAGESIZ);
    t_out.rdbuf()->sputc(static_cast<pt_num>(PageType::BTreeLeaf));
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&t_meta.m_n_rows),
                         sizeof(t_meta.m_n_rows));
    t_out.rdbuf()->sputn(std::bit_cast<const char*>(&t_meta.m_first_free),
                         sizeof(t_meta.m_first_free));
}

void read_from_impl(BTreeLeafMeta& t_meta, std::istream& t_in) {
    t_in.seekg(t_meta.m_page_num * PAGESIZ);
    auto pagetype = static_cast<uint8_t>(t_in.rdbuf()->sbumpc());
    if (pagetype != static_cast<pt_num>(PageType::BTreeLeaf)) {
        // I should return something more meaningful here.
        return;
    }
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&t_meta.m_n_rows),
                        sizeof(t_meta.m_n_rows));
    t_in.rdbuf()->sputn(std::bit_cast<char*>(&t_meta.m_first_free),
                        sizeof(t_meta.m_first_free));
}

} // namespace tinydb::dbfile
