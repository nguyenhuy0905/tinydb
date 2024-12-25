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
    using ss = std::streamsize;
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
    std::ifstream file{t_path, std::ios::binary};
    std::stringstream sbuilder{};

    file.get(*sbuilder.rdbuf(), '{');
    std::string tblname{};
    sbuilder >> tblname;
    sbuilder.clear();
    sbuilder.str(std::string{});
    file.seekg(1, std::ios_base::cur);
    TableMeta new_tbl{std::move(tblname)};
    EntrySiz off{0};

    while (file.peek() != '}') {
        file.get(*sbuilder.rdbuf(), ',');
        std::string colname{};
        file.seekg(1, std::ios_base::cur);
        sbuilder >> colname;
        sbuilder.clear();
        sbuilder.str(std::string{});

        file.get(*sbuilder.rdbuf(), ',');
        ColID id{0};
        file.seekg(1, std::ios_base::cur);
        sbuilder >> id;
        sbuilder.clear();
        sbuilder.str(std::string{});

        file.get(*sbuilder.rdbuf(), ';');
        EntrySiz size{0};
        file.seekg(1, std::ios_base::cur);
        sbuilder >> size;
        sbuilder.clear();
        sbuilder.str(std::string{});

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
