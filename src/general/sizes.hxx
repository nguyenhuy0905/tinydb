#ifndef TINYDB_GENERAL_SIZES_HXX
#define TINYDB_GENERAL_SIZES_HXX

#include <cstdint>

namespace tinydb {

constexpr uint16_t SIZEOF_PAGE = 4096;
constexpr uint16_t SIZEOF_VERSION_NUM = 2;
constexpr uint16_t SIZEOF_FILESIZ = 4;
constexpr uint16_t SIZEOF_FREELIST_PTR = 4;
constexpr uint16_t SIZEOF_HEAP = 4;

} // namespace tinydb

#endif // !TINYDB_GENERAL_SIZES_HXX
