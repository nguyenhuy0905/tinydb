#ifdef TINYDB_MODULE
module;
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fmt/color.h>
#include <fmt/format.h>
#ifndef TINYDB_IMPORT_STD
#include <concepts>
#include <expected>
#include <memory>
#include <span>
#include <utility>
#include <variant>
#include <vector>
#else
import std;
#endif // !TINYDB_IMPORT_STD
export module tinydb.stmt.parse;
import tinydb.stmt.token;
#else
#include <algorithm> // IWYU pragma: keep
#include <cassert>
#include <cstddef>
#include <expected>
#include <fmt/color.h>
#include <fmt/format.h>
#include <span>
#include <utility>
#endif // TINYDB_MODULE
#include "parse.detail.hpp"
#include "parse.hpp"

namespace tinydb::stmt {
// AST

[[nodiscard]] auto UnaryExprAst::eval() const -> EvalRet {
  using enum UnaryOp;
  switch (m_op) {
  case Plus:
    return m_data.eval();
  case Minus:
    // HACK: assumption that m_data is int64_t if it's in an unary expr.
    // I will need a more complicated EvalRet in order to handle different types
    // of numerics, and deliver an error when it's not a number.
    return -1 * std::get<int64_t>(m_data.eval());
  }
}

[[nodiscard]] auto UnaryExprAst::clone() const -> UnaryExprAst {
  return UnaryExprAst{m_op, m_data.clone()};
}

[[nodiscard]] auto UnaryExprAst::format() const -> std::string {
  return fmt::format(
      "(unary-expr: (unary-op: {}) {})",
      [this]() {
        using enum UnaryOp;
        switch (m_op) {
        case Plus:
          return '+';
        case Minus:
          return '-';
        }
      }(),
      m_data.format());
}

[[nodiscard]] auto MulExprAst::eval() const -> EvalRet {
  // TODO: better error handling some day
  assert(std::holds_alternative<int64_t>(m_first_expr.eval()));
  int64_t retval = std::get<int64_t>(m_first_expr.eval());
  for (const auto &un_expr : m_follow_exprs) {
    assert(std::holds_alternative<int64_t>(m_first_expr.eval()));
    using enum MulOp;
    switch (un_expr.first) {
    case Multiply:
      retval *= std::get<int64_t>(un_expr.second.eval());
      break;
    case Divide:
      assert(std::get<int64_t>(un_expr.second.eval()) != 0);
      retval /= std::get<int64_t>(un_expr.second.eval());
    }
  }
  return retval;
}

[[nodiscard]] auto MulExprAst::clone() const -> MulExprAst {
  return MulExprAst{m_first_expr, m_follow_exprs};
}

[[nodiscard]] auto MulExprAst::format() const -> std::string {
  return fmt::format("(mul-expr: {}{})", m_first_expr.format(), [&]() {
    std::string ret{};
    for (const auto &un_expr : m_follow_exprs) {
      ret.append(fmt::format(
          "\n\t(mul-op: {}) {}",
          [&]() {
            switch (un_expr.first) {
              using enum MulOp;
            case Multiply:
              return '*';
            case Divide:
              return '/';
            }
          }(),
          un_expr.second.format()));
    }
    return ret;
  }());
}

static_assert(AstNode<NumberAst>);
static_assert(AstDump<NumberAst>);
static_assert(AstNode<StrAst>);
static_assert(AstDump<StrAst>);
static_assert(AstNode<UnaryExprAst>);
static_assert(AstDump<UnaryExprAst>);
static_assert(AstNode<MulExprAst>);
static_assert(AstDump<MulExprAst>);
static_assert(AstNode<AddExprAst>);
static_assert(AstDump<AddExprAst>);

// TODO: maybe create a ParseData class to hold data necessary for parsing. The
// parser here is just a state machine.

struct ParseData;

/**
 * @class Parser
 * @brief Yet another state machine.
 *
 */
class Parser {
public:
  auto operator()(ParseData &t_data) { return (*m_parse_func)(t_data); }

private:
  auto (*m_parse_func)(ParseData &) -> std::expected<Parser, ParseError>;
};

auto parse(std::span<Token> t_tokens) -> std::expected<ParseRet, ParseError> {
  [[maybe_unused]] auto stfu = t_tokens;
  fmt::print(fmt::fg(fmt::color::red), "Error: unimplemented!!!!!!!\n");
  std::unreachable();
}

} // namespace tinydb::stmt
