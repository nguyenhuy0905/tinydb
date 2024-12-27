#include "page.hxx"
#include <bit>

namespace tinydb::dbfile {

void FreePageMeta::write_to(std::ostream& t_out) {
    t_out.seekp(m_pagenum * PAGESIZ);

    t_out << static_cast<uint8_t>(0);
    // it seems that if I don't do this, it automatically writes one byte if
    // m_n_pages is 1 digit long, even though we want 4.
    //
    // the table metadata thing is simpler in the sense that, we can rely on the
    // delimiter (comma, semicolon) to determine how long a variable is.
    t_out.write(std::bit_cast<const char*>(&m_n_pages), sizeof(m_n_pages));
    t_out.write(std::bit_cast<const char*>(&m_p_next_page),
               sizeof(m_p_next_page));
}

void FreePageMeta::read_from(std::istream& t_in) {
    uint8_t pagetype{0};
    t_in >> pagetype;
    if (pagetype != static_cast<pt_num>(PageType::Free)) {
        // I doubt this is even recoverable, so let's throw an exception.
        throw PageReadError::WrongPageType;
    }

    // and, similarly, we need to rawdog the bytes into the number(s) here.
    uint32_t npages{0};
    t_in.rdbuf()->sgetn(std::bit_cast<char*>(&npages), sizeof(npages));

    uint32_t pnext{0};
    t_in.rdbuf()->sgetn(std::bit_cast<char*>(&pnext), sizeof(npages));

    m_n_pages = npages;
    m_p_next_page = pnext;
}

FreePageMeta::~FreePageMeta() = default;

} // namespace tinydb::dbfile
