#ifndef TINYDB_STMT_PARSE_HPP
#define TINYDB_STMT_PARSE_HPP

#include "tinydb_export.h"
#ifndef TINYDB_MODULE
#include "token.hpp"
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <span>
#include <string>
#include <variant>
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

using EvalRet = std::variant<int64_t, std::string>;

// specialize the following functions for each type of Ast node.

/**
 * @brief Evaluates the @ref Ast passed in into a value.
 *
 * @tparam T The type of node.
 * @param t_node The. Node.
 * @return Evaluation value.
 */
template <typename T> auto eval(T &t_node) -> EvalRet;
/**
 * @brief Biology.
 *
 * @tparam T The type of node.
 * @param t_node The node.
 * @return A clone of the node.
 */
template <typename T> auto clone(T &t_node) -> T;

/**
 * @concept AstNode
 * @brief Requires template-specializing the following functions:
 * - @ref eval
 * - @ref clone
 */
template <typename T>
concept AstNode = requires(T t_node) {
  { tinydb::stmt::eval<T>(t_node) } -> std::same_as<EvalRet>;
  { tinydb::stmt::clone<T>(t_node) } -> std::same_as<T>;
};

/**
 * @class Ast
 * @brief Type-erased syntax tree node.
 *
 * A syntax tree can have multiple nodes, or one node only, that's why this
 * takes the name @ref Ast but not @ref AstNode.
 */
class Ast {
public:
  explicit Ast(AstNode auto &&t_node)
      : m_impl{new AstModel<std::remove_cvref_t<decltype(t_node)>>} {}
  Ast(const Ast &t_other) : m_impl{t_other.m_impl->do_clone()} {}
  auto operator=(const Ast &t_other) -> Ast & {
    if (this == &t_other) {
      return *this;
    }
    this->m_impl = t_other.m_impl->do_clone();
    return *this;
  }
  Ast(Ast &&t_other) = default;
  auto operator=(Ast &&t_other) -> Ast & = default;
  ~Ast() = default;

  /**
   * @brief Evaluates @c this @ref Ast and returns the value.
   * @return The value.
   * @note If you know the concrete type of a node, prefer to use @ref eval.
   */
  auto do_eval() -> EvalRet { return m_impl->do_eval(); }
  /**
   * @brief Clones @c this node.
   * @return A fresh, new copy of @c this node.
   * @note If you know the concrete type of a node, prefer to use @ref clone.
   */
  auto do_clone() -> Ast { return Ast{m_impl->do_clone()}; }

private:
  struct AstConcept {
    virtual auto do_eval() -> EvalRet = 0;
    virtual auto do_clone() -> std::unique_ptr<AstConcept> = 0;

    virtual ~AstConcept() = default;
    AstConcept(const AstConcept &) = delete;
    AstConcept(AstConcept &&) = delete;
    auto operator=(const AstConcept &) -> AstConcept & = delete;
    auto operator=(AstConcept &&) -> AstConcept & = delete;
  };
  template <AstNode A> struct AstModel : AstConcept {
    auto do_eval() -> EvalRet override { return eval(m_data); }
    A m_data;
  };

  explicit Ast(std::unique_ptr<AstConcept> t_impl)
      : m_impl{std::move(t_impl)} {}

  std::unique_ptr<AstConcept> m_impl;
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
