/**
 * @file token.hpp
 * @brief Defines and tokenizes an input string into separate tokens.
 */

#ifndef TINYDB_STMT_TOKEN_HPP
#define TINYDB_STMT_TOKEN_HPP

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
  Colon,      // :
  Comma,      // ,
  Star,       // *
  Plus,       // +
  Minus,      // -
  Slash,      // /
  Dot,        // .
  Equal,      // =
  Greater,    // >
  Less,       // <
  Bang,       // !
  Ampersand,  // &
  Beam,       // |
  // more complex symbols
  EqualEqual,   // ==
  BangEqual,    // !=
  GreaterEqual, // >=
  LessEqual,    // <=
  AmAmpersand,  // &&
  BeamBeam,     // ||
  // literals
  String,     // inside double-quote
  Number,     // 1, 2, -3.4, ...
  Identifier, // looks like a string, but has no quote
  // keywords
  And,
  Or,
  Not,
  Let,
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
    Done, // is it really an error?
    // Can be an error, or we can just politely ask for more input.
    UnendedStmt,
    // Anything other than ```[a-zA-Z0-9(),;+-*/=!<>.]```,
    // aka, not:
    //   - alphanumeric
    //   - an integer/decimal -1 3.14 69e2
    //   - parentheses
    //   - arithmetics operator + - * /
    //   - comma and semicolon , ;
    //   - logical operation = ! < > != >= <= ==
    UnexpectedChar,
    // Only procc'ed when parsing.
    // Any language has some grammar rule, mofo.
    UnexpectedToken,
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
      size_t t_line) -> Token {
    return Token{.lexeme{}, .line = t_line, .type = TokenType::Null};
  }
  /**
   * @brief String representation of the token, e.g. "and"
   *
   * Before you complain about "heap allocation bad":
   * - Almost every token is expected to be rather small in size; say, about
   * fewer than 15 characters.
   * - That's when short-string optimization kicks in. You can imagine the
   * string being an union with 2 similar-size variants.
   * - If the total size of the string is small enough, it can fit inside the
   * union; hence, no heap allocation.
   */
  std::string lexeme;
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

#endif // !TINYDB_STMT_TOKEN_HPP
