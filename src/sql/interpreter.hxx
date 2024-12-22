#ifndef TINYDB_AST_HXX
#define TINYDB_AST_HXX

#include "tokenizer.hxx"
#include <span>

namespace tinydb {

/**
 * @brief Return codes of the interpret function, duh.
 * */
enum class InterpreterRetCode : uint8_t {
    Ok = 0,
    Err,
};

/**
 * @brief I was lazy with the interpreter. This shitty thing is good enough for
 * now.
 *
 * @param t_tokens Whatever Tokenizer spits out.
 */
auto interpret(std::span<Token> t_tokens) -> InterpreterRetCode;

} // namespace tinydb

#endif // !TINYDB_AST_HXX
