#ifndef TINYDB_STMT_PARSE_DETAIL_HPP
#define TINYDB_STMT_PARSE_DETAIL_HPP

#ifndef TINYDB_MODULE
#include "parse.hpp"
#include <cstddef>
#include <fmt/format.h>
#include <string>
#endif // !TINYDB_MODULE

#ifdef TINYDB_MODULE
export namespace tinydb::stmt {
#else
namespace tinydb::stmt {
#endif // TINYDB_MODULE

// literals

/**
 * @class NumberAst
 * @brief Holds a literal number. Evaluates to a literal number.
 *
 * Sometimes, life needn't be so complicated. You ask for a number, it gives a
 * number back. Think about it, these literal values are the building blocks to
 * create more complex expressions. Sometimes, you gotta appreciate the simple
 * things in life.
 */
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

/**
 * @class StrAst
 * @brief Holds an owning literal string. Evaluates to a literal string.
 *
 */
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

} // tinydb::stmt

#endif // !TINYDB_STMT_PARSE_DETAIL_HPP
