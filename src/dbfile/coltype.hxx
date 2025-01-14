#include "modules.hxx"
#include <cstddef>
#include <cstdint>
#include <span>
#include <variant>
#include <vector>
#ifndef ENABLE_MODULE

#endif // !ENABLE_MODULE

TINYDB_EXPORT
namespace tinydb::dbfile::column {

// TODO: I'm planning to move column type definitions to here. I don't want the
// client to have to depend on the table header file.

/**
 * @enum ScalarColType
 * @brief More like numeric types. But I digress.
 */
enum class ScalarColType : uint8_t {
    Int16,
    Uint16,
};

/**
 * @return The byte size of the specified scalar type.
 * @param t_type The specified scalar type.
 */
constexpr auto scalar_size(ScalarColType t_type) -> uint8_t {
    using enum ScalarColType;
    switch (t_type) {
    case Int16:
    case Uint16:
        return sizeof(uint16_t);
        // the more type we have, the more we need to add.
    }
}

/**
 * @class Text
 * @brief Haven't figured out a good name for this. Basically any type of
 * variadic-length data, for example, a string, or those audiophiles' FLAC
 * files.
 *
 */
class Text {
  public:
    /**
     * @return The underlying data.
     */
    [[nodiscard]] constexpr auto get_data() -> std::span<std::byte> {
        return {m_data.begin(), m_data.end()};
    }
    /**
     * @return The size of the underlying data.
     */
    [[nodiscard]] constexpr auto get_size() const -> uint64_t {
        return m_data.size();
    }

  private:
    std::vector<std::byte> m_data;
};

template <class... Ts> struct overload : Ts... {
    using Ts::operator()...;
};

using ColType = std::variant<ScalarColType, Text>;

/**
 * @return The size of the type.
 * @param t_type The type.
 */
constexpr auto type_size(const ColType& t_type) -> uint64_t {

    return std::visit(
        overload{[](ScalarColType t_scalar) {
                     return static_cast<uint64_t>(scalar_size(t_scalar));
                 },
                 [](const Text& t_txt) { return t_txt.get_size(); }},
        t_type);
}

}; // namespace tinydb::dbfile::column
