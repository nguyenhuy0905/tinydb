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

  auto retval = Ok;
  std::visit(matches{[&](const Identifier& id) {
                       if (id.val != "select") {
                         retval = Err;
                       };
                     },
                     [&](const Literal&) { retval = Err; },
                     [&](const Symbol&) { retval = Err; }},
             t_tokens[0]);
  return retval;
}

} // namespace tinydb
