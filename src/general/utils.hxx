#ifndef TINYDB_GENERAL_UTILS_HXX
#define TINYDB_GENERAL_UTILS_HXX

namespace tinydb {

// Support for std::visit
template <class... Ts> struct overload : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overload(Ts...) -> overload<Ts...>;

} // namespace tinydb

#endif // !TINYDB_GENERAL_UTILS_HXX
