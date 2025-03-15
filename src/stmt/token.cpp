#ifdef TINYDB_MODULE
module;
#include <cassert>
#include <cstddef>
#include <cstdint>
#ifndef TINYDB_IMPORT_STD
#include <algorithm>
#include <array>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#endif // !TINYDB_IMPORT_STD
export module tinydb.stmt:token;
#ifdef TINYDB_IMPORT_STD
import std;
#endif // TINYDB_IMPORT_STD
#else
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>
#endif // TINYDB_MODULE
#include "token.hpp"

namespace {
using namespace tinydb::stmt;

struct MapPair {
  std::string_view key;
  TokenType token;
};
static_assert(std::is_trivially_copyable_v<MapPair>);
// why the hell can constinit throw an exception though.

[[maybe_unused]] constexpr auto keyword_search(std::string_view t_kw)
    -> std::optional<MapPair> {
  [[maybe_unused]] static constinit const std::array keyword_lut =
      []() noexcept {
        std::array ret{
            MapPair{.key = "let", .token = TokenType::Let},
            MapPair{.key = "and", .token = TokenType::And},
            MapPair{.key = "or", .token = TokenType::Or},
            MapPair{.key = "not", .token = TokenType::Not},
        };
        std::ranges::sort(ret, std::less{}, &MapPair::key);
        return ret;
      }();

  int64_t left = 0;
  int64_t right = keyword_lut.size() - 1;
  int64_t mid = 0;
  while (left <= right) {
    mid = (left + right) / 2;
    const MapPair &mid_elem = keyword_lut.at(static_cast<size_t>(mid));
    if (mid_elem.key == t_kw) {
      return mid_elem;
    }
    if (mid_elem.key > t_kw) {
      left = static_cast<int64_t>(mid + 1);
      continue;
    }
    right = static_cast<int64_t>(mid - 1);
  }
  return std::nullopt;
}

enum struct TokenizeState : uint8_t {
  InitialState,
  IdentifierState,
  NumberState,
  StringState,
  LtState,
  GtState,
};

class TokenizerData {
public:
  auto move_token_list() && { return m_tokens; }

private:
  std::vector<Token> m_tokens;
  Token m_curr_token{};
  void add_token() { m_tokens.push_back(std::move(m_curr_token)); }
  friend class Tokenizer;
};

class Tokenizer {
public:
  Tokenizer() : m_tok_fn{tokenize_initial} {}
  auto operator()(TokenizerData &t_data, char t_c)
      -> std::expected<Tokenizer, ParseError>;

private:
  static auto tokenize_initial(TokenizerData &t_data, char t_c)
      -> std::expected<Tokenizer, ParseError>;
  static auto tokenize_identifier(TokenizerData &t_data, char t_c)
      -> std::expected<Tokenizer, ParseError>;
  static auto tokenize_number(TokenizerData &t_data, char t_c)
      -> std::expected<Tokenizer, ParseError>;
  static auto tokenize_string(TokenizerData &t_data, char t_c)
      -> std::expected<Tokenizer, ParseError>;
  static auto tokenize_lt(TokenizerData &t_data, char t_c)
      -> std::expected<Tokenizer, ParseError>;
  static auto tokenize_gt(TokenizerData &t_data, char t_c)
      -> std::expected<Tokenizer, ParseError>;

  std::expected<Tokenizer, ParseError> (*m_tok_fn)(TokenizerData &, char);
};
static_assert(std::is_trivially_copyable_v<Tokenizer>);

auto Tokenizer::tokenize_initial(TokenizerData &t_data, char t_c)
    -> std::expected<Tokenizer, ParseError> {
  // LSP shutting-up
  [[maybe_unused]] auto lsp_shut = std::make_tuple(t_data, t_c);
  std::unreachable();
  // TODO: impl all the tokenize functions
}

} // namespace

namespace tinydb::stmt {

auto tokenize(std::string_view t_sv)
    -> std::expected<std::vector<Token>, ParseError> {
  assert(t_sv == t_sv);

  return std::unexpected{
      ParseError{.pos = 0, .type = ParseError::ErrType::EmptyStmt}};
}

} // namespace tinydb::stmt
