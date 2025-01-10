#include "tbl.hxx"
#include <iostream>
#include <print>
#include <ranges>
#include <sstream>

namespace tinydb::dbfile {

void TableMeta::write_to(std::ostream& t_out) {
    t_out.seekp(TABLE_OFFSET);
    // table format:
    // tblname{col1-name,col1-id,col1-size;...;coln-name,coln-id,coln-size;}

    // id is NOT written as characters, due to how this thing works. uint8_t
    // gets interpreted as bytes. With this implementation, the names cannot
    // contain any comma, semicolon or curly brace.

    // maybe I should throw some kind of failbit exception here.
    t_out << m_name << '{';
    t_out << m_key << ';';
    for (const ColumnMeta& colmeta :
         m_entries | std::views::transform(
                         [](const auto& pair) { return pair.second; })) {
        t_out << colmeta.m_name << ',' << colmeta.m_col_id << ','
              << colmeta.m_size << ';';
    }
    t_out << '}';
}

auto TableMeta::read_from(std::istream& t_in) -> std::optional<TableMeta> {
    t_in.seekg(0);
    // read the comment at the beginning of the definition of `write_to` to get
    // a sense of how the table metadata is written.
    // maybe I should throw some kind of failbit exception here.
    std::stringstream sbuilder{};

    // Put data into the stringstream, until either EOF or the delimiter.
    // There's zero checking whether the file is in valid format.
    // After that, put formatted data into the variable passed in, then clear
    // the stringstream flags and content.
    auto fill_var = [&]<typename T>(char delim, T& var) mutable {
        t_in.get(*sbuilder.rdbuf(), delim);
        sbuilder >> var;
        sbuilder.clear();
        sbuilder.str(std::string{});
        t_in.seekg(1, std::ios_base::cur);
    };

    std::string tblname{};
    fill_var('{', tblname);
    TableMeta new_tbl{std::move(tblname)};

    std::string key{};
    fill_var(';', key);

    EntrySiz off{0};
    while (t_in.peek() != '}') {
        std::string colname{};
        fill_var(',', colname);

        ColID id{0};
        fill_var(',', id);

        EntrySiz size{0};
        fill_var(';', size);

        auto new_col = ColumnMeta{.m_name{std::move(colname)},
                                  .m_size = size,
                                  .m_col_id = id,
                                  .m_offset = off};
        new_tbl.add_column(std::move(new_col));
        off += size;
    }

    return new_tbl;
}

} // namespace tinydb::dbfile
