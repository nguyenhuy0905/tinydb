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

namespace {
using namespace tinydb::stmt;

template <typename... Vis> struct Visitor : Vis... {
  using Vis::operator()...;
};

/**
 * @brief Parses an expression. Duh.
 */
[[maybe_unused]] auto parse_expr(std::span<Token>, size_t)
    -> std::expected<std::pair<ExprAst, size_t>, ParseError>;
[[maybe_unused]] auto parse_add_expr(std::span<Token>, size_t)
    -> std::expected<std::pair<AddExprAst, size_t>, ParseError>;
[[maybe_unused]] auto parse_mul_expr(std::span<Token>, size_t)
    -> std::expected<std::pair<MulExprAst, size_t>, ParseError>;
[[maybe_unused]] auto parse_unary_expr(std::span<Token>, size_t)
    -> std::expected<std::pair<UnaryExprAst, size_t>, ParseError>;
[[maybe_unused]] auto parse_lit_expr(std::span<Token>, size_t)
    -> std::expected<std::pair<LitExprAst, size_t>, ParseError>;

} // namespace

namespace tinydb::stmt {
using namespace std::literals;
// AST

LitExprAst::LitExprAst(std::variant<NumberAst, StrAst, ExprAst> t_data)
    : m_data{std::visit(Visitor{[](const AstNode auto &t_data) {
                                  return decltype(m_data){t_data};
                                },
                                [](const ExprAst &t_data) {
                                  return decltype(m_data){
                                      std::make_unique<ExprAst>(t_data)};
                                }},
                        t_data)} {}

[[nodiscard]] auto LitExprAst::eval() const -> EvalRet {
  return std::visit(
      Visitor{[](const AstNode auto &t_data) { return t_data.eval(); },
              [](const std::unique_ptr<ExprAst> &t_data) {
                return t_data->eval();
              }},
      m_data);
}

[[nodiscard]] auto LitExprAst::clone() const -> LitExprAst { return {*this}; }

[[nodiscard]] auto LitExprAst::format() const -> std::string {
  return fmt::format(
      "(lit-exp: {})",
      std::visit(
          Visitor{[](const AstNode auto &t_data) { return t_data.format(); },
                  [](const std::unique_ptr<ExprAst> &t_data) {
                    return t_data->format();
                  }},
          m_data));
}

LitExprAst::LitExprAst(const LitExprAst &t_other)
    : m_data{
          std::visit(Visitor{[](const AstNode auto &t_data) {
                               return decltype(m_data){t_data};
                             },
                             [](const std::unique_ptr<ExprAst> &t_data) {
                               return decltype(m_data){
                                   std::make_unique<ExprAst>(t_data->clone())};
                             }},
                     t_other.m_data)} {}

auto LitExprAst::operator=(const LitExprAst &t_other) -> LitExprAst & {
  if (this == &t_other) {
    return *this;
  }
  m_data = std::visit(
      Visitor{
          [](const AstNode auto &t_data) { return decltype(m_data){t_data}; },
          [](const std::unique_ptr<ExprAst> &t_data) {
            return decltype(m_data){std::make_unique<ExprAst>(t_data->clone())};
          }},
      t_other.m_data);
  return *this;
}
UnaryExprAst::UnaryExprAst(UnOp t_op, LitExprAst t_lit)
    : m_lit{std::move(t_lit)}, m_op{t_op} {}

[[nodiscard]] auto UnaryExprAst::eval() const -> EvalRet {
  auto ret = m_lit.eval();
  // TODO: better err handling
  assert(std::holds_alternative<int64_t>(ret));
  switch (m_op) {
    using enum UnOp;
  case Plus:
    break;
  case Minus:
    std::get<int64_t>(ret) *= -1;
    break;
  }
  return ret;
}

[[nodiscard]] auto UnaryExprAst::clone() const -> UnaryExprAst {
  return {*this};
}

[[nodiscard]] auto UnaryExprAst::format() const -> std::string {
  char un_op = [&]() {
    switch (m_op) {
      using enum UnOp;
    case Plus:
      return '+';
    case Minus:
      return '-';
    }
  }();
  return fmt::format("(un-exp: (un-op: {}) {})", un_op, m_lit.format());
}
MulExprAst::MulExprAst(UnaryExprAst t_first, std::vector<MulGr> &&t_follows)
    : m_follow{std::move(t_follows)}, m_first{std::move(t_first)} {}

MulExprAst::MulExprAst(UnaryExprAst t_first) : m_first{std::move(t_first)} {}

[[nodiscard]] auto MulExprAst::eval() const -> EvalRet {
  assert(std::holds_alternative<int64_t>(m_first.eval()));
  auto ret = std::get<int64_t>(m_first.eval());
  for (const auto &[op, exp] : m_follow) {
    assert(std::holds_alternative<int64_t>(exp.eval()));
    switch (op) {
      using enum MulOp;
    case Mul:
      ret *= std::get<int64_t>(exp.eval());
      break;
    case Div:
      assert(std::get<int64_t>(exp.eval()) != 0);
      ret /= std::get<int64_t>(exp.eval());
      break;
    }
  }

  return ret;
}

[[nodiscard]] auto MulExprAst::clone() const -> MulExprAst { return {*this}; }

[[nodiscard]] auto MulExprAst::format() const -> std::string {
  return fmt::format("(mul-exp: {}{})", m_first.format(), [&]() {
    std::string ret{};
    for (const auto &[op, exp] : m_follow) {
      char c_op = [&]() {
        using enum MulOp;
        switch (op) {
        case Mul:
          return '*';
        case Div:
          return '/';
        }
      }();
      ret.append(fmt::format("\n\t(mul-op: {}) {}", c_op, exp.format()));
    }
    return ret;
  }());
}

AddExprAst::AddExprAst(MulExprAst t_first, std::vector<AddGr> &&t_follows)
    : m_follow{std::move(t_follows)}, m_first{std::move(t_first)} {}

AddExprAst::AddExprAst(MulExprAst t_first) : m_first{std::move(t_first)} {}

[[nodiscard]] auto AddExprAst::eval() const -> EvalRet {
  assert(std::holds_alternative<int64_t>(m_first.eval()));
  auto ret = std::get<int64_t>(m_first.eval());
  for (const auto &[op, exp] : m_follow) {
    assert(std::holds_alternative<int64_t>(exp.eval()));
    switch (op) {
      using enum AddOp;
    case Add:
      ret += std::get<int64_t>(exp.eval());
      break;
    case Sub:
      ret -= std::get<int64_t>(exp.eval());
      break;
    }
  }
  return ret;
}

[[nodiscard]] auto AddExprAst::clone() const -> AddExprAst { return {*this}; }

[[nodiscard]] auto AddExprAst::format() const -> std::string {
  return fmt::format("(mul-exp: {}{})", m_first.format(), [&]() {
    std::string ret{};
    for (const auto &[op, exp] : m_follow) {
      char c_op = [&]() {
        using enum AddOp;
        switch (op) {
        case Add:
          return '+';
        case Sub:
          return '-';
        }
      }();
      ret.append(fmt::format("\n\t(mul-op: {}) {}", c_op, exp.format()));
    }
    return ret;
  }());
}

ExprAst::ExprAst(AddExprAst t_add) : m_add{std::move(t_add)} {}

[[nodiscard]] auto ExprAst::eval() const -> EvalRet { return m_add.eval(); }

[[nodiscard]] auto ExprAst::clone() const -> ExprAst { return {*this}; }

[[nodiscard]] auto ExprAst::format() const -> std::string {
  return fmt::format("(expr: {})", m_add.format());
}

static_assert(AstNode<NumberAst>);
static_assert(AstDump<NumberAst>);
static_assert(AstNode<StrAst>);
static_assert(AstDump<StrAst>);
static_assert(AstNode<LitExprAst>);
static_assert(AstDump<LitExprAst>);
static_assert(AstNode<UnaryExprAst>);
static_assert(AstDump<UnaryExprAst>);
static_assert(AstNode<MulExprAst>);
static_assert(AstDump<MulExprAst>);
static_assert(AstNode<AddExprAst>);
static_assert(AstDump<AddExprAst>);
static_assert(AstNode<ExprAst>);
static_assert(AstDump<ExprAst>);

} // namespace tinydb::stmt

namespace {

auto parse_expr(std::span<Token> t_tokens, size_t t_idx)
    -> std::expected<std::pair<ExprAst, size_t>, ParseError> {
  auto &&ret = parse_add_expr(t_tokens, t_idx);
  if (!ret) {
    return std::unexpected{ret.error()};
  }
  return std::pair{ExprAst{ret.value().first}, ret.value().second};
}

auto parse_add_expr(std::span<Token> t_tokens, size_t t_idx)
    -> std::expected<std::pair<AddExprAst, size_t>, ParseError> {
  auto &&first_ret = parse_mul_expr(t_tokens, t_idx);
  if (!first_ret) {
    return std::unexpected{first_ret.error()};
  }
  std::vector<AddExprAst::AddGr> follows{};
  auto idx = first_ret.value().second;

  {
    using enum AddExprAst::AddOp;

    auto get_add_op = [=](size_t t_idx) -> std::optional<AddExprAst::AddOp> {
      switch (t_tokens[t_idx].type) {
      case TokenType::Plus:
        return Add;
        break;
      case TokenType::Minus:
        return Sub;
        break;
      default:
        return std::nullopt;
        break;
      }
    };
    auto add_op = get_add_op(idx);
    if (!add_op) {
      return std::pair{
          AddExprAst{std::move(first_ret->first), std::move(follows)}, idx};
    }

    auto mul_exp_pair = parse_mul_expr(t_tokens, idx);
    for (; mul_exp_pair && add_op;
         mul_exp_pair = parse_mul_expr(t_tokens, idx)) {
      follows.emplace_back(*add_op, mul_exp_pair->first);
      add_op = get_add_op(mul_exp_pair->second);
      idx = mul_exp_pair->second;
    }
    if (!mul_exp_pair) {
      return std::unexpected{mul_exp_pair.error()};
    }
  }

  return std::pair{AddExprAst{std::move(first_ret->first), std::move(follows)},
                   idx};
}

} // namespace

namespace tinydb::stmt {
auto parse(std::span<Token> t_tokens) -> std::expected<ParseRet, ParseError> {
  [[maybe_unused]] auto stfu = t_tokens;
  [[maybe_unused]] size_t tok_idx = 0;
  ParseRet ret{};
  return ret;
}

} // namespace tinydb::stmt
