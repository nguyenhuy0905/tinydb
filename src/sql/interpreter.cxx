#include "interpreter.hxx"
#include "tokenizer.hxx"
#include <cstdint>
#include <span>
#include <variant>

namespace tinydb {

/**
 * @brief Helper for variants
 *
 * @tparam Ts
 */
template <class... Ts> struct matches : Ts... {
    using Ts::operator()...;
};

enum class CommandState : uint8_t {
    Select,
};

auto interpret(std::span<Token> t_tokens) -> InterpreterRetCode {
    using enum InterpreterRetCode;
    if (t_tokens.empty()) {
        return Err;
    }

    using enum InterpreterRetCode;
    auto retval = Ok;
    std::visit(matches{[&](const Identifier& id) {
                           if (id.val != "select") {
                               retval = Err;
                           };
                       },
                       [&](const Literal& _) { retval = Err; },
                       [&](const Symbol& _) { retval = Err; }},
               t_tokens[0]);
    return retval;
}

} // namespace tinydb
