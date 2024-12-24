#include "tbl.hxx"
#include <filesystem>
#include <fstream>
#include <ios>
#include <ranges>

namespace tinydb::dbfile {

void TableMeta::write_to(const std::filesystem::path& path) {
    std::ofstream file{path, std::ios::binary | std::ios::trunc};
    file.exceptions(std::ofstream::failbit);

    file.write(m_name.c_str(), static_cast<std::streamsize>(m_name.length()))
        .put('{');

    for (const ColumnMeta& colmeta :
         m_entries | std::views::transform(
                         [](const auto pair) { return pair.second; })) {
        file.write(colmeta.m_name.c_str(),
                   static_cast<std::streamsize>(colmeta.m_name.length()))
            .put(' ')
            // this seems to be necessary to write binary into a file
            //NOLINTBEGIN(*-reinterpret-cast)
            .write(reinterpret_cast<const char*>(colmeta.m_col_id),
                   sizeof(colmeta.m_col_id));
            //NOLINTEND(*-reinterpret-cast)
    }
}
} // namespace tinydb::dbfile
