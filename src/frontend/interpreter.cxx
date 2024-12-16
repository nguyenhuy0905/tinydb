#include "interpreter.hxx"
#include <span>

namespace tinydb {

/**
 * @brief Helper for variants
 *
 * @tparam Ts
 */
template <class... Ts> struct matches : Ts... {
    using Ts::operator()...;
};

auto interpret(std::span<Token> t_tokens) -> InterpreterRetCode {
    using enum InterpreterRetCode;
    if(t_tokens.empty()) {
        return Err;
    }

    return Ok;
}

} // namespace tinydb
