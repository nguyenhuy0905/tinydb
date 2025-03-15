#ifndef TINYDB_DBFILE_STMT_TOKEN_DETAIL_HPP
#define TINYDB_DBFILE_STMT_TOKEN_DETAIL_HPP

#ifndef TINYDB_MODULE
#include <cassert>
#include <functional>
#include <string>
#include <ranges>
#include <string_view>
#endif
#include "token.hpp"

#ifdef TINYDB_MODULE
export namespace tinydb::stmt {
#else
namespace tinydb::stmt {
#endif

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

auto keyword_lookup(std::string_view t_sv) noexcept
    -> std::optional<TokenType>;

enum struct TokenizeState : uint8_t {
  InitialState,
  SymbolState,
  IdentifierState,
  NumberState,
  StringState,
  ExclState,
  LtState,
  GtState,
};

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
  Token m_curr_token = Token::new_empty_at(m_remain_str.cbegin(), 1);

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
    char ret = m_remain_str.front();
    m_remain_str = {m_remain_str | std::views::drop(1)};
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

    m_curr_token.lexeme = {m_curr_token.lexeme.cbegin(),
                           m_curr_token.lexeme.cend() + 1};
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
    auto next_str_iter = m_curr_token.lexeme.cend();
    auto line_num = (m_curr_token.type == TokenType::Semicolon)
                        ? m_curr_token.line + 1
                        : m_curr_token.line;
    m_tokens.push_back(m_curr_token);
    m_curr_token = Token::new_empty_at(next_str_iter, line_num);
    m_remain_str = {next_str_iter, m_remain_str.end()};
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
      -> std::expected<Tokenizer, ParseError>;

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

}

#endif
