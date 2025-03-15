/**
 * @file token.hpp
 * @brief Defines and tokenizes an input string into separate tokens.
 */

#ifndef TINYDB_REPL_TOKEN_HPP
#define TINYDB_REPL_TOKEN_HPP

#ifndef TINYDB_MODULE
#include <cstddef>
#include <cstdint>
#include <expected>
#include <string_view>
#include <vector>
#endif // !TINYDB_MODULE

#ifdef TINYDB_MODULE
export namespace tinydb::stmt {
#else
namespace tinydb::stmt {
#endif // TINYDB_MODULE

/**
 * @enum TokenType
 * @brief All the supported token types.
 */
enum struct TokenType : uint8_t {
  Null, // not tokenized yet; indeterminate state. Should never happen after
        // @ref tokenize
  Eof,  // end-of-file
  // single character stuff
  LeftParen,  // (
  RightParen, // )
  Semicolon,  // ;
  Comma,      // ,
  Star,       // *
  Plus,       // +
  Minus,      // -
  Slash,      // /
  // Dot,        // .
  Equal,      // =
  Greater,    // >
  Less,       // <
  // more complex symbols
  GreaterEqual, // >=
  LessEqual,    // <=
  // literals
  String,     // inside double-quote
  Number,     // 1, 2, -3.4, ...
  Identifier, // looks like a string, but has no quote
  // keywords
  And,
  Or,
  Not,
  Select,
  From,
  Where,
  Ya, // true
  Na, // false
};

/**
 * @class ParseError
 * @brief Return value when a parse error is encountered
 */
struct ParseError {
  enum struct ErrType : uint8_t {
    // Can be an error, or we can just politely ask for more input.
    UnendedStmt = 0,
    // Anything other than ```[a-zA-Z0-9(),;+-*/=!<>.]```,
    // aka, not:
    //   - alphanumeric
    //   - an integer/decimal -1 3.14 69e2
    //   - parentheses
    //   - arithmetics operator + - * /
    //   - comma and semicolon , ;
    //   - logical operation = ! < > != >= <= ==
    UnexpectedChar,
  };
  /**
   * @brief The type of failure.
   */
  ErrType type;
};

/**
 * @class Token
 * @brief Simply holds a token data.
 *
 */
struct Token {
  /**
   * @brief Constructs a default empty token starting at the specified position.
   *
   * @param t_p_pos The const_iterator to the starting position.
   * @param t_line The line number of the token.
   * @return A @ref Token. Duh.
   */
  static auto new_empty_at(
      // it's not required that const_iterator is a pointer.
      std::string_view::const_iterator t_pos, // NOLINT(*identifier-naming*)
      size_t t_line) -> Token {
    return Token{
        .lexeme{t_pos, t_pos}, .line = t_line, .type = TokenType::Null};
  }
  /**
   * @brief String representation of the token, e.g. "and"
   */
  std::string_view lexeme;
  /**
   * @brief Mainly for better error message.
   */
  size_t line;
  TokenType type;
};

/**
 * @brief Turns a string into a vector of tokens.
 *
 * @param t_sv The string.
 */
auto tokenize(std::string_view t_sv)
    -> std::expected<std::vector<Token>, ParseError>;
}

#endif // !TINYDB_REPL_TOKEN_HPP
