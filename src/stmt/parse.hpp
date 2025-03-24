#ifndef TINYDB_STMT_PARSE_HPP
#define TINYDB_STMT_PARSE_HPP

#include "tinydb_export.h"
#ifndef TINYDB_MODULE
#include "token.hpp"
#include <cstddef>
#include <expected>
#include <memory>
#include <span>
#endif // !TINYDB_MODULE

#ifdef TINYDB_MODULE
export namespace tinydb::stmt {
#else
namespace tinydb::stmt {
#endif // TINYDB_MODULE

/**
 * @class Ast
 * @brief The syntax tree.
 *
 */
class Ast;

/**
 * @class ParseRet
 * @brief Parse result. What else could it be?
 *
 */
struct ParseRet;

class Ast {
public:
  Ast(const Ast &) = delete;
  Ast(Ast &&) = delete;
  auto operator=(const Ast &) -> Ast & = delete;
  auto operator=(Ast &&) -> Ast & = delete;
  virtual ~Ast() = default;

  /**
   * @brief Parse parse parse.
   *
   * @param t_tokens The toke list.
   * @return @ref ParseRet if successful, certain @ref ParseError otherwise.
   */
  static auto parse(std::span<Token> t_tokens)
      -> std::expected<ParseRet, ParseError>;

protected:
  /**
   * @brief Parse parse parse. The private virtual interface.
   *
   * @param t_tokens The token list.
   * @return @ref ParseRet if successful, certain @ref ParseError otherwise.
   */
  virtual auto do_parse(std::span<Token> t_tokens)
      -> std::expected<ParseRet, ParseError> = 0;
};

/**
 * @class ParseRet
 * @brief Return value when parsing succeeded.
 *
 */
struct ParseRet {
  /**
   * @brief The parse tree.
   */
  std::unique_ptr<Ast> ast;
  /**
   * @brief The offset from the token list passed in.
   */
  size_t pos{};
};

/**
 * @brief Parses a list of @ref Token into syntax tree(s).
 *
 * @param t_tokens
 */
auto TINYDB_EXPORT parse(std::span<Token> t_tokens)
    -> std::expected<ParseRet, ParseError>;
}

#endif // !TINYDB_STMT_PARSE_HPP
