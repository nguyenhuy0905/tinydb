#ifndef TINYDB_DBFILE_TBL_HXX
#define TINYDB_DBFILE_TBL_HXX

#include <cstdint>
#include <string>
#include <unordered_map>

namespace tinydb::dbfile {

using ColID = uint8_t;
using EntrySiz = uint16_t;

struct TableEntry {
    std::string m_name;
    uint16_t m_size;
    ColID m_col_id;
};

class Table {
    std::unordered_map<ColID, TableEntry> m_entries;
};

}

#endif // !TINYDB_DBFILE_TBL_HXX
