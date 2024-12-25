#include "tbl.hxx"
#include <bit>
#include <filesystem>
#include <fstream>
#include <ios>
#include <print>
#include <ranges>
#include <sstream>

namespace tinydb::dbfile {

void TableMeta::write_to(const std::filesystem::path& path) {
    // table format:
    // tblname{col1-name,col1-id,col1-size;...;coln-name,coln-id,coln-size;}

    // id and size are NOT written as characters.
    // With this implementation, the names cannot contain any comma, semicolon
    // or curly brace.
    using ss = std::streamsize;
    // maybe I should throw some kind of failbit exception here.
    std::ofstream file{path, std::ios::binary | std::ios::trunc};
    file.write(m_name.c_str(), static_cast<ss>(m_name.length())).put('{');
    for (const ColumnMeta& colmeta :
         m_entries | std::views::transform(
                         [](const auto& pair) { return pair.second; })) {
        file.write(colmeta.m_name.c_str(),
                   static_cast<ss>(colmeta.m_name.length()))
            .put(',')
            .write(std::bit_cast<const char*>(&colmeta.m_col_id),
                   sizeof(colmeta.m_col_id))
            .put(',')
            .write(std::bit_cast<const char*>(&colmeta.m_size),
                   sizeof(colmeta.m_size))
            .put(';');
    }
    file.put('}');
}

auto TableMeta::read_from(const std::filesystem::path& t_path)
    -> std::optional<TableMeta> {
    // read the comment at the beginning of the definition of `write_to` to get
    // a sense of how the table metadata is written.
    std::ifstream file{t_path, std::ios::binary};
    // maybe I should throw some kind of failbit exception here.
    std::stringstream sbuilder{};

    // Put data into the stringstream, until either EOF or the delimiter.
    // There's zero checking whether the file is in valid format.
    // After that, put formatted data into the variable passed in, then clear
    // the stringstream flags and content.
    auto fill_var = [&]<typename T>(char delim, T& var) mutable {
        file.get(*sbuilder.rdbuf(), delim);
        sbuilder >> var;
        sbuilder.clear();
        sbuilder.str(std::string{});
        file.seekg(1, std::ios_base::cur);
    };

    std::string tblname{};
    fill_var('{', tblname);
    TableMeta new_tbl{std::move(tblname)};

    EntrySiz off{0};
    while (file.peek() != '}') {
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
