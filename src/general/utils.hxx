#ifndef TINYDB_GENERAL_UTILS_HXX
#define TINYDB_GENERAL_UTILS_HXX

#include <cstdint>

namespace tinydb {

// Support for std::visit
template <class... Ts> struct overload : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overload(Ts...) -> overload<Ts...>;

// Page pointer.
using page_ptr_t = uint32_t;
// Offset relative to the start of a page.
using page_off_t = uint16_t;


} // namespace tinydb

#endif // !TINYDB_GENERAL_UTILS_HXX
