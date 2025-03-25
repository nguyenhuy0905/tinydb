#ifdef TINYDB_MODULE
module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fmt/color.h>
#include <fmt/format.h>
#ifndef TINYDB_IMPORT_STD
#include <expected>
#include <memory>
#include <span>
#include <utility>
#include <variant>
#else
import std;
#endif // !TINYDB_IMPORT_STD
export module tinydb.stmt.parse;
import tinydb.stmt.token;
#else
#include <algorithm> // IWYU pragma: keep
#include <cstddef>
#include <expected>
#include <fmt/color.h>
#include <fmt/format.h>
#include <span>
#include <utility>
#endif // TINYDB_MODULE
#include "parse.hpp"

namespace tinydb::stmt {

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
