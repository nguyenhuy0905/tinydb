#ifndef ENABLE_MODULE
#include "dbfile/internal/tbl.hxx"
#include "dbfile/coltype.hxx"
#include "general/offsets.hxx"
#include <iostream>
#include <optional>
#include <print>
#include <ranges>
#include <sstream>
#else
module;
#include "general/offsets.hxx"
#include <cstdint>
export module tinydb.dbfile.internal.tbl;
import std;
#include "dbfile/internal/tbl.hxx"
#endif // !ENABLE_MODULE

namespace tinydb::dbfile::internal {

void TableMeta::write_to(std::ostream& t_out) {
    t_out.seekp(TBL_OFF);
    // table format:
    // tblname{col1-name,col1-idcol1-typecol1-sizecol1-offsetcol2-name,col2-id...coln-name,coln-id,coln-type,coln-size,coln-offset;}
    //
    // Basically:
    // - anything from TBL_OFF to before { is the table name.
    // - Anything after the { until before , is first column's name.
    // - id, type, size, offset are all numbers. So, the size is their bound.
    // - Anything after the last byte of column 1's offset and before the next ,
    // is column 2's name.
    // - Rinse and repeat from here.

    auto& rdbuf = *t_out.rdbuf();
    auto write = [&]<typename T>(const T& t_val) -> std::streamsize {
        return rdbuf.sputn(std::bit_cast<const char*>(&t_val), sizeof(T));
    };
    auto write_string = [&](const std::string& t_str) -> std::streamsize {
        return rdbuf.sputn(t_str.c_str(),
                           static_cast<std::streamsize>(t_str.length()));
    };

    write_string(m_name);
    write('{');
    write_string(m_key);
    write(';');
    for (const ColumnMeta& colmeta :
         m_entries | std::views::transform(
                         [](const auto& pair) { return pair.second; })) {
        auto type_id = column::type_id(colmeta.m_type);
        auto typesiz = column::type_size(colmeta.m_type);
        write_string(colmeta.m_name);
        rdbuf.sputc(',');
        // with this, I don't really need the commas anymore.
        // The numbers' bounds are their sizes.
        write(colmeta.m_col_id);
        write(type_id);
        write(typesiz);
        write(colmeta.m_offset);
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

    auto fill_string = [&](char delim) -> std::string {
        std::stringstream var{};
        while (t_in.peek() != delim) {
            var.put(static_cast<char>(t_in.get()));
        }
        t_in.seekg(1, std::ios_base::cur);
        return var.str();
    };

    auto fill_num =
        [&]<typename T, typename... Args>(Args... t_placeholders) -> T {
        T var{t_placeholders...};
        t_in.rdbuf()->sgetn(std::bit_cast<char*>(&var), sizeof(T));
        return var;
    };

    auto tblname = fill_string('{');
    TableMeta new_tbl{std::move(tblname)};

    auto key = fill_string(';');

    while (t_in.peek() != '}') {
        auto colname = fill_string(',');

        auto id = fill_num.operator()<ColID>();

        auto typenum = fill_num.operator()<column::coltype_num_t>();

        auto type = column::type_of(typenum).value();
        auto size = fill_num.operator()<uint64_t>();
        type = column::map_type(
            type, [](auto t_scl) { return column::ColType{t_scl}; },
            [&](column::TextType&) mutable {
                return column::ColType{column::TextType{size}};
            });

        auto off = fill_num.operator()<uint8_t>();

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
