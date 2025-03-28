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
 * @see AddExprAst
 * @note I'm not sure if I will use this one. For now, it's kind of just an
 * alias for @ref Ast.
 *
 * Definition (for now):
 * add-exp
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
 * @see LitExprAst
 *
 * Definition:
 * ('+' | '-')? lit-exp
 * (NUM preceded by one of '+', '-' or nothing at all)
 */
class UnaryExprAst;
/**
 * @class LitExprAst
 * @brief Evaluates to a literal expression. Aka, ```lit-exp```
 * @see NumberAst
 * @see StrAst
 * @see ExprAst
 *
 * Definition:
 * NUM | STR | '(' exp ')'
 */
class LitExprAst;
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

class LitExprAst {
public:
  LitExprAst(LitExprAst &&) = default;
  auto operator=(const LitExprAst &) -> LitExprAst &;
  auto operator=(LitExprAst &&) -> LitExprAst & = default;
  explicit LitExprAst(std::variant<NumberAst, StrAst, ExprAst> t_data);
  LitExprAst(const LitExprAst &);
  ~LitExprAst() = default;
  [[nodiscard]] auto eval() const -> EvalRet;
  [[nodiscard]] auto clone() const -> LitExprAst;
  [[nodiscard]] auto format() const -> std::string;

private:
  std::variant<NumberAst, StrAst, std::unique_ptr<ExprAst>> m_data;
};

class UnaryExprAst {
public:
  enum struct UnOp : uint8_t {
    Plus,
    Minus,
  };
  explicit UnaryExprAst(UnOp t_op, LitExprAst t_lit);
  [[nodiscard]] auto eval() const -> EvalRet;
  [[nodiscard]] auto clone() const -> UnaryExprAst;
  [[nodiscard]] auto format() const -> std::string;

private:
  LitExprAst m_lit;
  UnOp m_op;
};

class MulExprAst {
public:
  enum struct MulOp : bool {
    Mul,
    Div,
  };
  using MulGr = std::pair<MulOp, UnaryExprAst>;
  MulExprAst(UnaryExprAst t_first, std::vector<MulGr> &&t_follows);
  explicit MulExprAst(UnaryExprAst t_first);
  [[nodiscard]] auto eval() const -> EvalRet;
  [[nodiscard]] auto clone() const -> MulExprAst;
  [[nodiscard]] auto format() const -> std::string;

private:
  std::vector<MulGr> m_follow;
  UnaryExprAst m_first;
};

class AddExprAst {
public:
  enum struct AddOp : bool {
    Add,
    Sub,
  };
  using AddGr = std::pair<AddOp, MulExprAst>;
  AddExprAst(MulExprAst t_first, std::vector<AddGr> &&t_follows);
  explicit AddExprAst(MulExprAst t_first);
  [[nodiscard]] auto eval() const -> EvalRet;
  [[nodiscard]] auto clone() const -> AddExprAst;
  [[nodiscard]] auto format() const -> std::string;

private:
  std::vector<AddGr> m_follow;
  MulExprAst m_first;
};

class ExprAst {
public:
  explicit ExprAst(AddExprAst t_add);
  [[nodiscard]] auto eval() const -> EvalRet;
  [[nodiscard]] auto clone() const -> ExprAst;
  [[nodiscard]] auto format() const -> std::string;

private:
  AddExprAst m_add;
};

} // tinydb::stmt

#endif // !TINYDB_STMT_PARSE_DETAIL_HPP
