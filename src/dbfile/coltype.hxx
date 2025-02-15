/**
 * @file coltype.hxx
 * @brief Defines available column types and some convenient functions to deal
 * with column types.
 */

#ifndef TINYDB_DBFILE_COLTYPE_HXX
#define TINYDB_DBFILE_COLTYPE_HXX
#ifndef ENABLE_MODULES
#include "dbfile/heap_base.hxx"
#include "general/utils.hxx"
#include <cstdint>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#endif // !ENABLE_MODULES
#include "general/modules.hxx"

TINYDB_EXPORT
namespace tinydb::dbfile::column {

/**
 * @enum ScalarColType
 * @brief More like numeric types. But I digress.
 * @details Technically we could derive the underlying type as uint8_t which
 * saves more space. The only problem, we lose access to the entire formatting
 * output of streams.
 */
enum class ColType : char {
  Int8 = 0,
  Uint8,
  Int16,
  Uint16,
  Int32,
  Uint32,
  Int64,
  Uint64,
  Float32,
  Float64,
  Text,
};

// The column, but represented as in-memory data.
using InMemCol =
    std::variant<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t,
                 uint64_t, float, double, internal::Ptr>;

using coltype_num_t = std::underlying_type_t<ColType>;

/**
 * @param t_coltype The in-memory column value.
 * @return
 * - The contained value, if C is the right type.
 * - Throw std::bad_variant_access otherwise.
 * This is basically a wrapper around std::get.
 * Hence, cannot be used for runtime dispatch. For that purpose, use `get_if`
 * and run a check.
 */
template <ColType C> constexpr auto get_value(InMemCol t_coltype) {
  return std::get<static_cast<coltype_num_t>(C)>(t_coltype);
}

/**
 * @return the `ColType` enum value corresponding to the active variant
 * of `t_col`.
 */
[[nodiscard]] constexpr auto in_mem_col_type(InMemCol t_col) noexcept {
  using enum ColType;
  std::visit(overload{
                 [](int8_t) { return Int8; },
                 [](uint8_t) { return Uint8; },
                 [](int16_t) { return Int16; },
                 [](uint16_t) { return Uint16; },
                 [](int32_t) { return Int32; },
                 [](uint32_t) { return Uint32; },
                 [](int64_t) { return Int64; },
                 [](uint64_t) { return Uint64; },
                 [](float) { return Float32; },
                 [](double) { return Float64; },
                 [](internal::Ptr) { return Text; },
             },
             t_col);
}

// just to check if the template above is correct.
// constexpr int8_t TEST{get_value<ColType::Int8>({static_cast<int8_t>(1)})};

/**
 * @return The byte size of the specified scalar type.
 * @param t_type The specified scalar type.
 */
constexpr auto type_size(ColType t_type) -> uint8_t {
  using enum ColType;
  switch (t_type) {
  case Int8:
  case Uint8:
    return sizeof(uint8_t);
  case Int16:
  case Uint16:
    return sizeof(uint16_t);
  case Int32:
  case Uint32:
    return sizeof(uint32_t);
  case Int64:
  case Uint64:
    return sizeof(uint64_t);
  case Float32:
    return sizeof(float);
  case Float64:
    return sizeof(double);
  case Text:
    return internal::Ptr::SIZE;
    // the more type we have, the more we need to add.
  default:
    std::unreachable();
  }
}

/**
 * @param t_type The ColType passed in.
 * @return The type id of the specified column type.
 */
constexpr auto type_id(const ColType& t_type) -> coltype_num_t {
  return static_cast<coltype_num_t>(t_type);
}

/**
 * @brief Basically the reverse of type_id.
 *
 * @param t_num The numeric representation of a column type.
 * @return The appropriate column type.
 */
constexpr auto type_of(coltype_num_t t_num) -> std::optional<ColType> {
  if (t_num >= 0 && t_num <= static_cast<coltype_num_t>(ColType::Text)) {
    return static_cast<ColType>(t_num);
  }
  return std::nullopt;
}

} // namespace tinydb::dbfile::column

#endif // !TINYDB_DBFILE_COLTYPE_HXX
