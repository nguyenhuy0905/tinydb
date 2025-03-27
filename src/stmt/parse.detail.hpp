#ifndef TINYDB_STMT_PARSE_DETAIL_HPP
#define TINYDB_STMT_PARSE_DETAIL_HPP

#include "parse.hpp"
#ifndef TINYDB_MODULE
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <fmt/format.h>
#include <string>
#include <utility>
#include <vector>
#endif // !TINYDB_MODULE

#ifdef TINYDB_MODULE
export namespace tinydb::stmt {
#else
namespace tinydb::stmt {
#endif // TINYDB_MODULE

/**
 * @class ExprAst
 * @brief Top-level expression. Aka, ```exp```.
 * @see NumberAst
 * @see StrAst
 * @see AddExprAst
 * @see MulExprAst
 * @see UnaryExprAst
 * @note I'm not sure if I will use this one. For now, it's kind of just an
 * alias for @ref Ast.
 *
 * Definition:
 * NUM | STR | add-exp | un-exp | '(' exp ')'
 */
class ExprAst;
/**
 * @class AddExprAst
 * @brief Addition expression. Aka, ```add-exp```
 * @see MulExprAst
 *
 * Definition:
 * mul-exp (('+' | '-') mul-exp)*
 * (one or more mul-exp, with '+' or '-' in between each mul-exp)
 */
class AddExprAst;
/**
 * @class MulExprAst
 * @brief Multiplication expression, aka, ```mul-exp```
 * @see UnaryExprAst
 *
 * Definition:
 * un-exp (('*' | '/') un-exp)*
 * (one or more un-exp, with '*' or '/' in between each one)
 */
class MulExprAst;
/**
 * @class UnaryExprAst
 * @brief Unary expression, aka, ```un-exp```
 * @see NumberAst
 *
 * Definition:
 * ('+' | '-')? NUM
 * (NUM preceded by one of '+', '-' or nothing at all)
 */
class UnaryExprAst;
/**
 * @class NumberAst
 * @brief Holds a literal number. Evaluates to a literal number. Aka, ```NUM```.
 *
 * Sometimes, life needn't be so complicated. You ask for a number, it gives a
 * number back. Think about it, these literal values are the building blocks to
 * create more complex expressions. Sometimes, you gotta appreciate the simple
 * things in life.
 */
class NumberAst;
/**
 * @class StrAst
 * @brief Holds an owning literal string. Evaluates to a literal string. Aka,
 * ```STR```.
 *
 */
class StrAst;

class NumberAst {
public:
  explicit NumberAst(int64_t t_num) : m_num(t_num) {}
  [[nodiscard]] auto eval() const -> EvalRet { return m_num; }
  [[nodiscard]] auto clone() const -> NumberAst { return NumberAst{m_num}; }
  [[nodiscard]] auto format() const -> std::string {
    return fmt::format("(lit-num: {})", m_num);
  }

private:
  int64_t m_num;
};

class StrAst {
public:
  template <typename S>
  explicit StrAst(S &&t_str)
    requires std::convertible_to<S, std::string_view>
      : m_str{std::forward<S>(t_str)} {}
  [[nodiscard]] auto eval() const -> EvalRet { return m_str; }
  [[nodiscard]] auto clone() const -> StrAst { return StrAst{m_str}; }
  [[nodiscard]] auto format() const -> std::string {
    return fmt::format("(lit-str: {})", m_str);
  }

private:
  std::string m_str;
};

class UnaryExprAst {
public:
  /**
   * @brief Less error-prone than just @ref TokenType.
   */
  enum struct UnaryOp : uint8_t {
    Plus,
    Minus,
    // more to come
  };
  UnaryExprAst(UnaryOp t_op, NumberAst t_data) : m_data{t_data}, m_op{t_op} {}
  [[nodiscard]] auto eval() const -> EvalRet;
  [[nodiscard]] auto clone() const -> UnaryExprAst;
  [[nodiscard]] auto format() const -> std::string;

private:
  NumberAst m_data;
  UnaryOp m_op;
};

class MulExprAst {
public:
  enum struct MulOp : bool {
    Multiply,
    Divide,
  };
  using UnExprGroup = std::pair<MulOp, UnaryExprAst>;

  /**
   * @note Some possible, (hopefully) working ways to construct:
   *
   * ```cpp
   *
   * UnaryExprAst some_unary_expr{...};
   * std::vector<MulExprAst::UnExprGroup> some_vec{...};
   * ...
   * MulExprAst{some_unary_expr, std::from_range_t, some_vec}
   * MulExprAst{some_unary_expr, {MulExprAst::UnExprGroup{...}, ...}}
   * MulExprAst{some_unary_expr, {std::pair{...}, ...}}
   *
   * ```
   */
  template <typename... Args>
    requires std::constructible_from<std::vector<UnExprGroup>, Args...>
  explicit MulExprAst(UnaryExprAst t_un_expr, Args &&...t_args)
      : m_follow_exprs{std::forward<Args>(t_args)...}, m_first_expr{t_un_expr} {
  }
  [[nodiscard]] auto eval() const -> EvalRet;
  [[nodiscard]] auto clone() const -> MulExprAst;
  [[nodiscard]] auto format() const -> std::string;

private:
  std::vector<UnExprGroup> m_follow_exprs;
  UnaryExprAst m_first_expr;
};

class AddExprAst {
public:
  enum struct AddOp : bool {
    Plus,
    Minus,
  };
  using MulExprGroup = std::pair<AddOp, MulExprAst>;

  template <typename... Args>
    requires std::constructible_from<std::vector<MulExprGroup>, Args...>
  explicit AddExprAst(MulExprAst t_mul_expr, Args &&...t_args)
      : m_follow_exprs{std::forward<Args>(t_args)...},
        m_first_expr{std::move(t_mul_expr)} {};
  [[nodiscard]] auto eval() const -> EvalRet;
  [[nodiscard]] auto clone() const -> AddExprAst;
  [[nodiscard]] auto format() const -> std::string;

private:
  std::vector<MulExprGroup> m_follow_exprs;
  MulExprAst m_first_expr;
};

} // tinydb::stmt

#endif // !TINYDB_STMT_PARSE_DETAIL_HPP
