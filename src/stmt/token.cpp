#ifdef TINYDB_MODULE
module;
#include <cassert>
#include <cstddef>
#include <cstdint>
#ifndef TINYDB_IMPORT_STD
#include <cctype>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#endif // !TINYDB_IMPORT_STD
export module tinydb.stmt:token;
#ifdef TINYDB_IMPORT_STD
import std;
#endif // TINYDB_IMPORT_STD
#else
#include <cassert>
#include <cctype>
#include <cstddef>
#include <functional>
#include <optional>
#include <ranges>
#include <string_view>
#include <unordered_map>
#include <vector>
#endif // TINYDB_MODULE
#include "token.detail.hpp"
#include "token.hpp"

namespace tinydb::stmt {

auto keyword_lookup(std::string_view t_sv) noexcept
    -> std::optional<TokenType> {
  using namespace std::literals;
  static const std::unordered_map<std::string_view, TokenType,
                                  TransparentStrHash, std::equal_to<>>
      lookup{{"and"sv, TokenType::And},   {"or"sv, TokenType::Or},
             {"not"sv, TokenType::Not},   {"select"sv, TokenType::Select},
             {"from"sv, TokenType::From}, {"where"sv, TokenType::Where},
             {"ya"sv, TokenType::Ya},     {"na"sv, TokenType::Na}};
  auto ret = lookup.find(t_sv);
  if (ret == lookup.end()) {
    return std::nullopt;
  }
  return (*ret).second;
}

auto tokenize(std::string_view t_sv)
    -> std::expected<std::vector<Token>, ParseError> {
  assert(t_sv == t_sv);

  return std::unexpected{ParseError{.type = ParseError::ErrType::UnendedStmt}};
}

auto Tokenizer::tokenize_initial(TokenizerData &t_data)
    -> std::expected<Tokenizer, ParseError> {
  if (!t_data.peek_next_char()) {
    return std::unexpected{
        ParseError{.type = ParseError::ErrType::UnendedStmt}};
  }
  // why the cast:
  // https://en.cppreference.com/w/cpp/string/byte/isalpha
  auto peek_char = static_cast<unsigned char>(*t_data.peek_next_char());
  if (peek_char == static_cast<unsigned char>(' ')) {
    t_data.pop_next_char();
    return Tokenizer{tokenize_initial};
  }
  if (std::isalpha(peek_char) == 0) {
    return Tokenizer{tokenize_identifier};
  }
  if (std::isdigit(peek_char) == 0) {
    return Tokenizer{tokenize_number};
  }
  return Tokenizer{tokenize_symbol};
}

auto Tokenizer::tokenize_symbol(TokenizerData &t_data)
    -> std::expected<Tokenizer, ParseError> {
  if (!t_data.peek_next_char()) {
    return std::unexpected{
        ParseError{.type = ParseError::ErrType::UnendedStmt}};
  }
  auto peek_char = static_cast<unsigned char>(*t_data.peek_next_char());
  if (peek_char == static_cast<unsigned char>(' ')) {
    return Tokenizer{tokenize_symbol};
  }
  t_data.finish_current_token();
  assert(t_data.is_token_empty());

  switch (peek_char) {
  // for parentheses:
  // - whatever token was being built, is expected to be done at this point.
  case '"': {
    t_data.set_token_type(TokenType::String);
    t_data.pop_next_char();

    return Tokenizer{tokenize_string};
  }
  case '(': {
    t_data.set_token_type(TokenType::LeftParen);
    break;
  }
  case ')': {
    t_data.set_token_type(TokenType::RightParen);
    break;
  }
  case '+': {
    t_data.set_token_type(TokenType::Plus);
    break;
  }
  case '-': {
    t_data.set_token_type(TokenType::Minus);
    break;
  }
  case '*': {
    t_data.set_token_type(TokenType::Star);
    break;
  }
  case '/': {
    t_data.set_token_type(TokenType::Slash);
    break;
  }
  case ',': {
    t_data.set_token_type(TokenType::Comma);
    break;
  }
  case ';': {
    t_data.set_token_type(TokenType::Semicolon);
    break;
  }
  case '=': {
    t_data.set_token_type(TokenType::Equal);
    break;
  }
  case '>': {
    t_data.set_token_type(TokenType::Greater);
    t_data.add_next_char();
    t_data.finish_current_token();
    assert(t_data.is_token_empty());
    return Tokenizer{tokenize_gt};
  }
  case '<': {
    t_data.set_token_type(TokenType::Less);
    t_data.add_next_char();
    t_data.finish_current_token();
    assert(t_data.is_token_empty());
    return Tokenizer{tokenize_lt};
  }
  default:
    return std::unexpected{ParseError{ParseError::ErrType::UnexpectedChar}};
  }

  t_data.add_next_char();
  t_data.finish_current_token();
  assert(t_data.is_token_empty());
  return Tokenizer{tokenize_initial};
}

auto Tokenizer::tokenize_identifier(TokenizerData &t_data)
    -> std::expected<Tokenizer, ParseError> {
  if (!t_data.peek_next_char()) {
    return std::unexpected{
        ParseError{.type = ParseError::ErrType::UnendedStmt}};
  }
  auto peek_char = static_cast<unsigned char>(*t_data.peek_next_char());
  if (std::isalpha(peek_char) == 0) {
    t_data.add_next_char();
    if (t_data.is_null_token()) {
      t_data.set_token_type(TokenType::Identifier);
    }

    return Tokenizer{tokenize_identifier};
  }
  if (std::isdigit(peek_char) == 0) {
    assert(t_data.m_curr_token.type == TokenType::Identifier &&
           !t_data.is_null_token());
    t_data.add_next_char();
    return Tokenizer{tokenize_identifier};
  }
  switch (peek_char) {
  case '_':
    t_data.add_next_char();
    return Tokenizer{tokenize_identifier};
  default:
    auto keyword = keyword_lookup({t_data.m_curr_token.lexeme});
    if (keyword) {
      t_data.change_token_type(*keyword);
    }
    [[maybe_unused]] bool token_added = t_data.finish_current_token();
    assert(token_added);
    return Tokenizer{tokenize_symbol};
  }
}

} // namespace tinydb::stmt
