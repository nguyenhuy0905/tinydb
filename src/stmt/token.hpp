/**
 * @file token.hpp
 * @brief Defines and tokenizes an input string into separate tokens.
 */

#ifndef TINYDB_REPL_TOKEN_HPP
#define TINYDB_REPL_TOKEN_HPP

#ifndef TINYDB_MODULE
#include <cstddef>
#include <cstdint>
#include <string>
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
  Eof, // end-of-file
  // single character stuff
  LeftParen,  // (
  RightParen, // )
  Semicolon,  // ;
  Comma,      // ,
  Star,       // *
  Plus,       // +
  Minus,      // -
  Slash,      // /
  Dot,        // .
  Colon,      // :
  // literals
  String,     // inside single or double-quote
  Number,     // 1, 2, -3.4, ...
  Identifier, // looks like a string, but has no quote
  // keywords
  Let,
  Select,
  From,
  Where,
  And,
  Or,
  Not,
};

/**
 * @class Token
 * @brief Simply holds a token data.
 *
 */
struct Token {
  std::pmr::string lexeme;
  size_t line;
  TokenType type;
};

/**
 * @brief Turns a string into a vector of tokens.
 *
 * @param t_sv The string.
 */
auto tokenize(std::string_view t_sv) -> std::pmr::vector<Token>;
}

#endif // !TINYDB_REPL_TOKEN_HPP
