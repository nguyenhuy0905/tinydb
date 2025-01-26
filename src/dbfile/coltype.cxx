module;
#include <cstdint>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
export module tinydb.dbfile.coltype;

export namespace tinydb {

template <class... Ts> struct overload : Ts... {
    using Ts::operator()...;
};
template <class... Ts> overload(Ts...) -> overload<Ts...>;

}

export namespace tinydb::dbfile::column {

// TODO: I'm planning to move column type definitions to here. I don't want the
// client to have to depend on the table header file.

/**
 * @enum ScalarColType
 * @brief More like numeric types. But I digress.
 * @details Technically we could derive the underlying type as uint8_t which
 * saves more space. The only problem, we lose access to the entire formatting
 * output of streams.
 * So, if I want it that way, I need to write everything using raw rdbuf.
 */
enum class ScalarColType : uint16_t {
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
};

using coltype_num_t = std::underlying_type_t<ScalarColType>;

/**
 * @return The byte size of the specified scalar type.
 * @param t_type The specified scalar type.
 */
constexpr auto scalar_size(ScalarColType t_type) -> uint8_t {
    using enum ScalarColType;
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
        // the more type we have, the more we need to add.
    default:
        std::unreachable();
    }
}

/**
 * @class Text
 * @brief Haven't figured out a good name for this. Basically any type of
 * variadic-length data, for example, a string, or those audiophiles' FLAC
 * files.
 *
 */
struct TextType {
    static constexpr coltype_num_t TYPE_ID =
        static_cast<std::underlying_type_t<ScalarColType>>(
            ScalarColType::Float64) +
        1;
    /**
     * @return The size of the underlying data.
     */
    [[nodiscard]] constexpr auto get_size() const -> uint64_t { return m_size; }

    uint64_t m_size;
};

using ColType = std::variant<ScalarColType, TextType>;

template <typename F>
concept FScalarMap = requires(F f, ScalarColType st) { std::invoke(f, st); };

template <typename F>
concept FTextMap = requires(F f, TextType tt) { std::invoke(f, tt); };

/**
 * @brief Applies the correct map function to the type variant passed in.
 * Both functions t_f_scalar and t_f_text should return the same type.
 *
 * @tparam Col Anything of ColType. Can be ColType, const ColType& or ColType&&,
 * or whatever qualifiers there are.
 * @param t_type The variant
 * @param t_f_scalar Callable object/function to operate on if column type holds
 * a scalar type.
 * @param t_f_text Callable object/function to operate on if column type holds
 * the text type.
 * @return The return type of t_f_scalar and t_f_text.
 */
template <class Col>
    requires std::is_same_v<std::remove_cvref_t<Col>, ColType>
constexpr auto map_type(Col& t_type, FScalarMap auto&& t_f_scalar,
                        FTextMap auto&& t_f_text) -> decltype(auto) {
    // std::invoke nicely handles a couple extra cases, eg, if a function
    // pointer is passed, I need to deref the pointer then call the function in
    // the syntax (*f)(args...).
    return std::visit(
        overload{[&](ScalarColType t_scalar) {
                     return std::invoke(t_f_scalar, t_scalar);
                 },
                 // TextType& and const TextType& are different things,
                 // so just let auto find the right type.
                 [&](auto& t_txt) { return std::invoke(t_f_text, t_txt); }},
        t_type);
}

/**
 * @return The size of the type.
 * @param t_type The type.
 *
 * @details The return value is 64 bytes to accomodate the actual size of
 * TextType.
 */
constexpr auto type_size(const ColType& t_type) -> uint64_t {
    return map_type(
        t_type,
        [](ScalarColType t_scalar) {
            return static_cast<uint64_t>(scalar_size(t_scalar));
        },
        [](const TextType& t_txt) { return t_txt.get_size(); });
}

/**
 * @param t_type The ColType passed in.
 * @return The type id of the specified column type.
 */
constexpr auto type_id(const ColType& t_type) -> coltype_num_t {
    return map_type(
        t_type,
        [](ScalarColType t_scalar) {
            return static_cast<coltype_num_t>(t_scalar);
        },
        [](const TextType&) { return TextType::TYPE_ID; });
}

/**
 * @brief Basically the reverse of type_id.
 *
 * @param t_num The numeric representation of a column type.
 * @return The appropriate column type.
 *   - For TextType, the struct contains a TextType of size 1 as
 *   the default.
 *   - If t_num isn't a valid numeric representation of any column type,
 *   std::nullopt is returned.
 */
constexpr auto type_of(coltype_num_t t_num) -> std::optional<ColType> {
    using enum ScalarColType;
    if (t_num >= type_id(Int8) && t_num <= type_id(Float64)) {
        return ScalarColType{t_num};
    }
    if (t_num == TextType::TYPE_ID) {
        return TextType{1};
    }
    return std::nullopt;
}

}; // namespace tinydb::dbfile::column
