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
#include <string_view>
#include <unordered_map>
#include <vector>
#endif // TINYDB_MODULE
#include "token.hpp"

namespace {

using namespace tinydb::stmt;

struct TransparentStrHash {
  // NOLINTBEGIN(*identifier-naming*)
  using hash_type = std::hash<std::string_view>;
  using is_transparent = void;
  // NOLINTEND(*identifier-naming*)

  auto operator()(const std::string &t_str) const -> std::size_t {
    return hash_type{}(t_str);
  }
  auto operator()(const char *t_p_str) const -> std::size_t {
    return hash_type{}(t_p_str);
  }
  auto operator()(std::string_view t_str) const -> std::size_t {
    return hash_type{}(t_str);
  }
};

auto keyword_lookup(std::string_view t_sv) noexcept -> std::optional<TokenType>;

/**
 * @class TokenizerData
 * @brief Necessary information for @ref Tokenizer to do its job.
 *
 */
class TokenizerData {
public:
  explicit TokenizerData(std::string_view t_sv)
      : m_remain_str{t_sv}, m_curr_token{} {}
  /**
   * @brief Consumes the current @ref TokenizerData instance, and returns its
   * token list.
   *
   * @return The token list. Duh.
   */
  constexpr auto move_token_list() && { return m_tokens; }

private:
  std::vector<Token> m_tokens;
  std::string_view m_remain_str;
  Token m_curr_token = Token::new_empty_at(1);

  [[nodiscard]] constexpr auto is_null_token() const -> bool {
    return m_curr_token.type == TokenType::Null;
  }

  [[nodiscard]] constexpr auto is_token_empty() const -> bool {
    return m_curr_token.lexeme.empty();
  }

  constexpr auto pop_next_char() -> std::optional<char> {
    if (m_remain_str.empty()) {
      return std::nullopt;
    }
    auto ret = m_remain_str.front();
    m_remain_str.remove_prefix(1);
    return ret;
  }

  [[nodiscard]] constexpr auto peek_next_char() const -> std::optional<char> {
    if (m_remain_str.empty()) {
      return std::nullopt;
    }
    return m_remain_str.front();
  }

  [[nodiscard]] constexpr auto get_begin_iter() const
      -> std::string_view::const_iterator {
    return m_remain_str.cbegin();
  }

  auto add_next_char() -> bool {
    if (m_remain_str.empty()) {
      return false;
    }

    m_curr_token.lexeme.push_back(m_remain_str.front());
    m_remain_str.remove_prefix(1);
    return true;
  }

  /**
   * @brief Use when you want to set a Null token into something else.
   */
  constexpr auto set_token_type(TokenType t_type) -> bool {
    if (!is_null_token()) {
      return false;
    }
    m_curr_token.type = t_type;
    return true;
  }

  /**
   * @brief Use when you want to set a non-Null token into something else.
   * If you set a Null token this way, nothing goes wrong, but @ref
   * set_token_type is designed for that.
   */
  constexpr void change_token_type(TokenType t_type) {
    m_curr_token.type = t_type;
  }

  constexpr auto finish_current_token() -> bool {
    if (is_null_token()) {
      return false;
    }

    // it's not required that const_iterator is a pointer.
    // NOLINTNEXTLINE(*qualified-auto*)
    auto line_num = (m_curr_token.type == TokenType::Semicolon)
                        ? m_curr_token.line + 1
                        : m_curr_token.line;
    m_tokens.push_back(m_curr_token);
    m_curr_token = Token::new_empty_at(line_num);
    return true;
  }
  friend class Tokenizer;
};

/**
 * @class Tokenizer
 * @brief A functor that builds the token list inside @ref TokenizerData.
 *
 * Internally contains a state machine.
 */
class Tokenizer {
public:
  Tokenizer() : m_tok_fn{tokenize_initial} {}
  auto operator()(TokenizerData &t_data)
      -> std::expected<Tokenizer, ParseError> {
    return (*m_tok_fn)(t_data);
  }

private:
  explicit Tokenizer(
      std::expected<Tokenizer, ParseError> (*t_p_tok_fn)(TokenizerData &))
      : m_tok_fn(t_p_tok_fn) {}
  static auto tokenize_initial(TokenizerData &t_data)
      -> std::expected<Tokenizer, ParseError>;
  static auto tokenize_symbol(TokenizerData &t_data)
      -> std::expected<Tokenizer, ParseError>;
  static auto tokenize_identifier(TokenizerData &t_data)
      -> std::expected<Tokenizer, ParseError>;
  static auto tokenize_number(TokenizerData &t_data)
      -> std::expected<Tokenizer, ParseError>;
  static auto tokenize_string(TokenizerData &t_data)
      -> std::expected<Tokenizer, ParseError>;
  static auto tokenize_lt(TokenizerData &t_data)
      -> std::expected<Tokenizer, ParseError>;
  static auto tokenize_gt(TokenizerData &t_data)
      -> std::expected<Tokenizer, ParseError>;

  std::expected<Tokenizer, ParseError> (*m_tok_fn)(TokenizerData &);
};
static_assert(std::is_trivially_copyable_v<Tokenizer>);
} // namespace

namespace tinydb::stmt {

auto tokenize(std::string_view t_sv)
    -> std::expected<std::vector<Token>, ParseError> {
  TokenizerData data{t_sv};
  auto ret = [&]() mutable {
    auto ret = std::expected<Tokenizer, ParseError>{Tokenizer{}};
    while (ret) {
      ret = (*ret)(data);
    }
    return ret;
  }();
  if (ret.error().type != ParseError::ErrType::Done) {
    return std::unexpected{ret.error()};
  }

  return std::move(data).move_token_list();
}

} // namespace tinydb::stmt

namespace {

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

auto Tokenizer::tokenize_initial(TokenizerData &t_data)
    -> std::expected<Tokenizer, ParseError> {
  if (!t_data.peek_next_char()) {
    if (!t_data.m_tokens.empty() &&
        t_data.m_tokens.back().type == TokenType::Semicolon) {
      return std::unexpected{ParseError{ParseError::ErrType::Done}};
    }
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
  if (std::isalpha(peek_char) != 0) {
    return Tokenizer{tokenize_identifier};
  }
  if (std::isdigit(peek_char) != 0) {
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
    t_data.finish_current_token();
    t_data.pop_next_char();
    return Tokenizer{tokenize_initial};
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
    return Tokenizer{tokenize_gt};
  }
  case '<': {
    t_data.set_token_type(TokenType::Less);
    t_data.add_next_char();
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
  if (std::isalpha(peek_char) != 0) {
    t_data.add_next_char();
    if (t_data.is_null_token()) {
      t_data.set_token_type(TokenType::Identifier);
    }

    return Tokenizer{tokenize_identifier};
  }
  if (std::isdigit(peek_char) != 0) {
    assert(t_data.m_curr_token.type == TokenType::Identifier &&
           !t_data.is_null_token());
    t_data.add_next_char();
    return Tokenizer{tokenize_identifier};
  }
  if (peek_char == '_') {
    t_data.add_next_char();
    return Tokenizer{tokenize_identifier};
  }
  auto keyword = keyword_lookup({t_data.m_curr_token.lexeme});
  if (keyword) {
    t_data.change_token_type(*keyword);
  }
  [[maybe_unused]] bool token_added = t_data.finish_current_token();
  assert(token_added);
  return Tokenizer{tokenize_symbol};
}

auto Tokenizer::tokenize_number(TokenizerData &t_data)
    -> std::expected<Tokenizer, ParseError> {
  if (!t_data.peek_next_char()) {
    return std::unexpected{
        ParseError{.type = ParseError::ErrType::UnendedStmt}};
  }
  auto peek_char = static_cast<unsigned char>(*t_data.peek_next_char());
  if (t_data.is_null_token()) {
    t_data.set_token_type(TokenType::Number);
  }
  if (std::isdigit(peek_char) != 0) {
    t_data.add_next_char();
    return Tokenizer{tokenize_number};
  }
  if (peek_char == ' ') {
    assert(t_data.m_curr_token.type == TokenType::Number);
    t_data.pop_next_char();
    t_data.finish_current_token();
    return Tokenizer{tokenize_initial};
  }
  if (peek_char == '.') {
    auto dotpos = t_data.m_curr_token.lexeme.find('.');
    if (dotpos != std::string_view::npos) {
      return std::unexpected{ParseError{ParseError::ErrType::UnexpectedChar}};
    }
    t_data.add_next_char();
    return Tokenizer{tokenize_number};
  }
  // deal with e later
  if (std::isalpha(peek_char) != 0) {
    return std::unexpected{ParseError{ParseError::ErrType::UnexpectedChar}};
  }
  // some sort of symbol here. That's not really related to this current number
  t_data.finish_current_token();
  return Tokenizer{tokenize_symbol};
}

auto Tokenizer::tokenize_string(TokenizerData &t_data)
    -> std::expected<Tokenizer, ParseError> {
  if (!t_data.peek_next_char()) {
    return std::unexpected{
        ParseError{.type = ParseError::ErrType::UnendedStmt}};
  }
  auto peek_char = static_cast<unsigned char>(*t_data.peek_next_char());
  if (t_data.is_null_token()) {
    t_data.set_token_type(TokenType::String);
  }
  if (peek_char == '"') {
    assert(t_data.m_curr_token.type == TokenType::String);
    t_data.pop_next_char();
    t_data.finish_current_token();
    return Tokenizer{tokenize_initial};
  }
  t_data.add_next_char();
  return Tokenizer{tokenize_string};
}

auto Tokenizer::tokenize_lt(TokenizerData &t_data)
    -> std::expected<Tokenizer, ParseError> {
  assert(t_data.m_curr_token.type == TokenType::Less);
  assert(t_data.m_curr_token.lexeme == "<");
  if (!t_data.peek_next_char()) {
    return std::unexpected{
        ParseError{.type = ParseError::ErrType::UnendedStmt}};
  }
  auto peek_char = static_cast<unsigned char>(*t_data.peek_next_char());

  if (peek_char == '=') {
    t_data.change_token_type(TokenType::LessEqual);
    t_data.add_next_char();
  }
  t_data.finish_current_token();
  return Tokenizer{tokenize_initial};
}

auto Tokenizer::tokenize_gt(TokenizerData &t_data)
    -> std::expected<Tokenizer, ParseError> {
  assert(t_data.m_curr_token.type == TokenType::Greater);
  assert(t_data.m_curr_token.lexeme == ">");
  if (!t_data.peek_next_char()) {
    return std::unexpected{
        ParseError{.type = ParseError::ErrType::UnendedStmt}};
  }
  auto peek_char = static_cast<unsigned char>(*t_data.peek_next_char());

  if (peek_char == '=') {
    t_data.change_token_type(TokenType::GreaterEqual);
    t_data.add_next_char();
  }
  t_data.finish_current_token();
  return Tokenizer{tokenize_initial};
}

} // namespace
