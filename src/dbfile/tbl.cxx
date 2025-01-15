#ifndef ENABLE_MODULE
#include "tbl.hxx"
#include "coltype.hxx"
#include "offsets.hxx"
#include <iostream>
#include <optional>
#include <print>
#include <ranges>
#include <sstream>
#else
module;
#include "offsets.hxx"
#include <cstdint>
export module tinydb.dbfile.tbl;
import std;
#include "tbl.hxx"
#endif // !ENABLE_MODULE

// TODO: change all reads and writes to use rdbuf instead of the stream
// formatting.

namespace tinydb::dbfile {

void TableMeta::write_to(std::ostream& t_out) {
    t_out.seekp(TBL_OFF);
    // table format:
    // tblname{col1-name,col1-id,col1-type,col1-size,col1-offset;...;coln-name,coln-id,coln-type,coln-size,coln-offset;}

    // id is NOT written as characters, due to how this thing works. uint8_t
    // gets interpreted as bytes. With this implementation, the names cannot
    // contain any comma, semicolon or curly brace.

    // maybe I should throw some kind of failbit exception here.
    t_out << m_name << '{' << m_key << ';';
    for (const ColumnMeta& colmeta :
         m_entries | std::views::transform(
                         [](const auto& pair) { return pair.second; })) {
        auto type_id = column::type_id(colmeta.m_type);
        auto typesiz = column::type_size(colmeta.m_type);
        t_out << colmeta.m_name << ',' << colmeta.m_col_id << ',' << type_id
              << ',' << typesiz << ',' << colmeta.m_offset << ';';
    }
    t_out << '}';
}

auto TableMeta::read_from(std::istream& t_in) -> TableMeta {
    t_in.seekg(TBL_OFF);
    // read the comment at the beginning of the definition of `write_to` to get
    // a sense of how the table metadata is written.
    // maybe I should throw some kind of failbit exception here.

    // Put data into the stringstream, until either EOF or the delimiter.
    // There's zero checking whether the file is in valid format.
    // After that, put formatted data into the variable passed in, then clear
    // the stringstream flags and content.

    auto fill_var = [&]<typename T, typename... Args>(char delim,
                                                      Args... t_args) -> T {
        std::stringstream sbuilder{};
        T var{t_args...};
        t_in.get(*sbuilder.rdbuf(), delim);
        sbuilder >> var;
        t_in.seekg(1, std::ios_base::cur);
        return var;
    };

    auto tblname = fill_var.operator()<std::string>('{');
    TableMeta new_tbl{std::move(tblname)};

    auto key = fill_var.operator()<std::string>(';');

    while (t_in.peek() != '}') {
        auto colname = fill_var.operator()<std::string>(',');

        auto id = fill_var.operator()<ColID>(',');

        auto typenum = fill_var.operator()<column::IdType>(',');

        auto type = column::type_of(typenum).value();
        auto size = fill_var.operator()<uint64_t>(',');
        type = column::map_type(
            type, [](auto t_scl) { return column::ColType{t_scl}; },
            [&](column::TextType& _) mutable {
                return column::ColType{column::TextType{size}};
            });

        auto off = fill_var.operator()<uint8_t>(';');

        auto new_col = ColumnMeta{.m_name{std::move(colname)},
                                  .m_type = type,
                                  .m_col_id = id,
                                  .m_offset = off};
        new_tbl.add_column(std::move(new_col));
    }
    new_tbl.set_key(std::move(key));

    return new_tbl;
}

} // namespace tinydb::dbfile
