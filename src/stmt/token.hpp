/**
 * @file token.hpp
 * @brief Defines and tokenizes an input string into separate tokens.
 */

#ifndef TINYDB_REPL_TOKEN_HPP
#define TINYDB_REPL_TOKEN_HPP

#ifndef TINYDB_MODULE
#include <cstdint>
#endif // !TINYDB_MODULE

#ifdef TINYDB_MODULE
export namespace tinydb::stmt {
#else
namespace tinydb::stmt {
#endif // TINYDB_MODULE

enum struct TokenType : uint8_t {
  // single character stuff
  LeftParen,  // (
  RightParen, // )
  Semicolon,  // ;
  Comma,      // ,
  Star,       // *
  Plus,       // +
  Minus,      // -
  Slash,      // /
  // literals
  String,
  Number,
  // keywords
  Select,
  From,
  Where,
  And,
  Or,
  Not,
};
}

#endif // !TINYDB_REPL_TOKEN_HPP
