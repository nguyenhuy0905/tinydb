#ifndef TINYDB_REPL_STMT_HPP
#define TINYDB_REPL_STMT_HPP

#include "tinydb_export.h"
#ifndef TINYDB_MODULE
#include "token.hpp"
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <span>
#include <string_view>
#include <vector>
#endif // !TINYDB_MODULE

#ifdef TINYDB_MODULE
export namespace tinydb::stmt {
#else
namespace tinydb::stmt {
#endif // TINYDB_MODULE
/**
 * @class Statement
 * @brief Compiled from a string
 */
class TINYDB_EXPORT Statement {
public:
  /**
   * @class EvalResult
   * @brief Result of calling eval.
   *
   * For now, data is just some raw bytes.
   */
  struct EvalResult {
    std::vector<uint8_t> raw_bytes;
  };

  struct EvalError {
    enum struct ErrType : uint8_t {
      Undefined = 1,
    };
    /**
     * @brief The type of failure.
     */
    ErrType type;
  };

  Statement() = delete;
  static auto parse(std::string_view t_sv)
      -> std::expected<Statement, ParseError>;
  auto eval() -> std::expected<EvalResult, EvalError> { return m_stmt->eval(); }

private:
  /**
   * @class Ast
   * @brief The abstract syntax tree. Duh.
   */
  struct Ast { // NOLINT(*special-member-function*)
    virtual ~Ast() = default;
    /**
     * @return A clone of the derived struct of Ast.
     */
    auto clone() -> std::unique_ptr<Ast> { return do_clone(); }
    // TODO: think of a good return type for parse().

    /**
     * @brief Parses the tokens.
     *
     * @param t_tokens The tokens.
     * @return The remaining tokens.
     */
    auto parse(std::span<std::string_view> t_tokens)
        -> std::expected<std::span<std::string_view>, ParseError> {
      return do_parse(t_tokens);
    }

    /**
     * @brief Evaluates the statement.
     *
     * @return The evaluation result, or an error.
     */
    auto eval() -> std::expected<EvalResult, EvalError> { return do_eval(); }

  protected:
    Ast() = default;
    virtual auto do_clone() -> std::unique_ptr<Ast> = 0;
    virtual auto do_eval() -> std::expected<EvalResult, EvalError> = 0;
    virtual auto do_parse(std::span<std::string_view> t_tokens)
        -> std::expected<std::span<std::string_view>, ParseError> = 0;
  };
  /**
   * @brief The parsed statement.
   */
  std::unique_ptr<Ast> m_stmt;
};
} // namespace tinydb::repl

#endif // !TINYDB_REPL_STMT_HPP
