#include "page.hxx"
#include <bit>
#include <iostream>

namespace tinydb::dbfile {

void FreePageMeta::write_to(std::ostream& t_out) {
    t_out.seekp(m_pagenum * PAGESIZ);

    t_out << static_cast<pt_num>(PageType::Free);
    // it seems that if I don't do this, it automatically writes one byte if
    // m_n_pages is 1 digit long, even though we want 4.
    //
    // the table metadata thing is simpler in the sense that, we can rely on the
    // delimiter (comma, semicolon) to determine how long a variable is.
    t_out.write(std::bit_cast<const char*>(&m_p_next_page),
                sizeof(m_p_next_page));
}

void FreePageMeta::read_from(std::istream& t_in) {
    t_in.seekg(m_pagenum * PAGESIZ);

    uint8_t pagetype{0};
    t_in >> pagetype;
    if (pagetype != static_cast<pt_num>(PageType::Free)) {
        // I doubt this is even recoverable, so let's throw an exception.
        throw PageReadError::WrongPageType;
    }

    // and, similarly, we need to rawdog the bytes into the number(s) here.
    uint32_t pnext{0};
    t_in.rdbuf()->sgetn(std::bit_cast<char*>(&pnext), sizeof(pnext));

    m_p_next_page = pnext;
}

void BTreeLeafMeta::write_to(std::ostream& t_out) {
    t_out.seekp(m_pagenum * PAGESIZ);

    t_out << static_cast<pt_num>(PageType::BTreeLeaf);
    t_out.write(std::bit_cast<const char*>(&m_n_cols), sizeof(m_n_cols));
    t_out.write(std::bit_cast<const char*>(&m_p_next), sizeof(m_p_next));
}

void BTreeLeafMeta::read_from(std::istream& t_in) {
    t_in.seekg(m_pagenum * PAGESIZ);

    uint8_t pagetype{0};
    t_in >> pagetype;
    if (pagetype != static_cast<pt_num>(PageType::BTreeLeaf)) {
        throw PageReadError::WrongPageType;
    }

    uint16_t ncols{0};
    t_in.rdbuf()->sgetn(std::bit_cast<char*>(&ncols), sizeof(ncols));

    uint32_t pnextpg{0};
    t_in.rdbuf()->sgetn(std::bit_cast<char*>(&pnextpg), sizeof(pnextpg));

    m_n_cols = ncols;
    m_p_next = pnextpg;
}

void BTreeInternalMeta::write_to(std::ostream& t_out) {
    t_out.seekp(m_pagenum * PAGESIZ);
    t_out << static_cast<pt_num>(PageType::BTreeInternal);
    t_out.write(std::bit_cast<const char*>(&m_n_pairs), sizeof(m_n_pairs));
}

void BTreeInternalMeta::read_from(std::istream& t_in) {
    t_in.seekg(m_pagenum * PAGESIZ);

    uint8_t pagetype{0};
    t_in >> pagetype;
    if (pagetype != static_cast<pt_num>(PageType::BTreeInternal)) {
        throw PageReadError::WrongPageType;
    }

    uint16_t npairs{0};
    t_in >> npairs;

    m_n_pairs = npairs;
}

} // namespace tinydb::dbfile
