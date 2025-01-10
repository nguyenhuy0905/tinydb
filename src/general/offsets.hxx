#ifndef TINYDB_GENERAL_OFFSETS_HXX
#define TINYDB_GENERAL_OFFSETS_HXX

#include "sizes.hxx"
#include <cstdint>

namespace tinydb {

constexpr uint16_t VERSION_MAJOR_OFF = 0;
constexpr uint16_t VERSION_MINOR_OFF =
    VERSION_MAJOR_OFF + SIZEOF_VERSION_NUM; // 2
constexpr uint16_t VERSION_PATCH_OFF =
    VERSION_MINOR_OFF + SIZEOF_VERSION_NUM; // 4
constexpr uint16_t DBFILE_SIZE_OFF =
    VERSION_PATCH_OFF + SIZEOF_VERSION_NUM;                               // 6
constexpr uint16_t FREELIST_PTR_OFF = DBFILE_SIZE_OFF + SIZEOF_FILESIZ;   // 10
constexpr uint16_t HEAP_PTR_OFF = FREELIST_PTR_OFF + SIZEOF_FREELIST_PTR; // 14
constexpr uint16_t TBL_OFF = HEAP_PTR_OFF + SIZEOF_HEAP_PTR;              // 18

}; // namespace tinydb

#endif // !TINYDB_GENERAL_OFFSETS_HXX
