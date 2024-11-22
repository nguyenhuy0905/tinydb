#include "tokenizer.hxx"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>

namespace tinydb {

/**
 * @brief Whether this character is a symbol.
 *
 * @param c
 */
static auto is_symbol(char c) -> bool;

/**
 * @brief Helper for variants
 *
 * @tparam Ts
 */
template <class... Ts> struct matches : Ts... {
    using Ts::operator()...;
};

auto Tokenizer::parse(std::string_view input) -> ParserReturn {
    auto is_space = [](char c) { return c == ' ' || c == '\t' || c == '\n'; };
    // that scary views expression trims start and end spaces
    for (const char c :
         input | std::views::drop_while(is_space) | std::views::reverse |
             std::views::drop_while(is_space) | std::views::reverse) {
        auto exp{handle_state(c)};
        if (!exp.has_value()) {
            return exp;
        }
    }

    if (!m_curr_word.empty()) {
        m_tokens.emplace_back(std::move(m_curr_word));
    } else if (m_curr_num.has_value()) {
        m_tokens.emplace_back(m_curr_num.value());
    }

    return {};
}

auto Tokenizer::handle_word(char c) -> ParserReturn {
    if (c == ' ') {
        if (m_curr_word.empty()) {
            return {};
        }
        m_tokens.emplace_back(std::move(m_curr_word));
        // reconstruct m_curr_word
        m_curr_word = {};
        return {};
    }
    // a number must start with, well, a number.
    if (isdigit(c) && m_curr_word.empty()) {
        m_curr_num = c - '0';
        m_curr_state = State::Number;
        return {};
    }
    switch (c) {
    case '=':
        if (m_curr_word.empty()) {
            return {};
        }
        m_tokens.emplace_back(std::move(m_curr_word));
        m_curr_word = {};
        m_tokens.emplace_back(Symbol::Equal);
        m_curr_state = State::Equal;
        return {};
    case '<':
        if(!m_curr_word.empty()) {
            m_tokens.emplace_back(std::move(m_curr_word));
            m_curr_word = {};
        }
        m_curr_state = State::LessThan;
        return {};
    case '>':
        if(!m_curr_word.empty()) {
            m_tokens.emplace_back(std::move(m_curr_word));
            m_curr_word = {};
        }
        m_curr_state = State::MoreThan;
        return {};
    case '"':
        m_curr_state = State::Quote;
        return {};
    default:
        break;
    }
    m_curr_word.push_back(c);
    return {};
}

auto Tokenizer::handle_quote(char c) -> ParserReturn {
    if (c == '"') {
        if (m_curr_word.empty()) {
            return std::unexpected{ParseError::EmptyQuote};
        }
        m_tokens.emplace_back(std::move(m_curr_word));
        m_curr_word = {};
        m_curr_state = State::Word;
        return {};
    }

    m_curr_word.push_back(c);
    return {};
}

auto Tokenizer::handle_number(char c) -> ParserReturn {
    assert(m_curr_word.empty());

    if (c >= '0' && c <= '9') {
        // to avoid "magic number" warning.
        constexpr int64_t ten = 10;
        m_curr_num =
            m_curr_num
                .transform([c](int64_t num) { return num * ten + (c - '0'); })
                .or_else([c]() { return std::optional<int64_t>{c - '0'}; });
        return {};
    }
    if (c == ' ') {
        if (!m_curr_num.has_value()) {
            return {};
        }
        m_tokens.emplace_back(m_curr_num.value());
        m_curr_num.reset();
        m_curr_state = State::Word;
        return {};
    }
    m_curr_word = std::to_string(m_curr_num.value());
    if(c != '"') {
        m_curr_word.push_back(c);
    }
    m_curr_num.reset();
    m_curr_state = State::Word;

    return {};
}

auto Tokenizer::handle_equal(char c) -> ParserReturn {
    assert(m_curr_word.empty() && !m_curr_num.has_value());

    if(is_symbol(c)) {
        return std::unexpected{ParseError::SussySymbols};
    }
    if(isdigit(c)) {
        m_curr_num = c - '0';
        m_curr_state = State::Number;
        return {};
    }
    if(c == '"') {
        m_curr_state = State::Quote;
        return {};
    }
    if(c == ' ') {
        return {};
    }
    m_curr_state = State::Word;
    m_curr_word.push_back(c);

    return {};
}

auto Tokenizer::handle_less_than(char c) -> ParserReturn {
    assert(m_curr_word.empty());

    if (c == '=') {
        m_tokens.emplace_back(Symbol::LessThanEqual);
        m_curr_state = State::Equal;
        return {};
    }
    if (is_symbol(c)) {
        return std::unexpected{ParseError::SussySymbols};
    }
    m_tokens.emplace_back(Symbol::LessThan);
    m_curr_state = State::Word;

    return {};
}

auto Tokenizer::handle_more_than(char c) -> ParserReturn {
    assert(m_curr_word.empty());

    if (c == '=') {
        m_tokens.emplace_back(Symbol::MoreThanEqual);
        m_curr_state = State::Equal;
        return {};
    }
    if (is_symbol(c)) {
        return std::unexpected{ParseError::SussySymbols};
    }
    m_tokens.emplace_back(Symbol::LessThan);
    m_curr_state = State::Word;

    return {};

}

#ifndef NDEBUG

/**
 * @brief Return debug prints from a symbol.
 *
 * @param s
 * @return
 */
static auto format_symbol(Symbol s) -> std::string {
    std::string ret{"Symbol: "};
    switch (s) {
    case Symbol::Equal:
        ret.append("=");
        break;
    case Symbol::MoreThan:
        ret.append(">");
        break;
    case Symbol::LessThan:
        ret.append("<");
        break;
    case Symbol::Backslash:
        ret.append("\\");
        break;
    case Symbol::EqualEquals:
        ret.append("==");
        break;
    case Symbol::LessThanEqual:
        ret.append("<=");
        break;
    case Symbol::MoreThanEqual:
        ret.append(">=");
        break;
    }

    return ret;
}

static auto is_symbol(char c) -> bool {
    return c == '=' || c == '<' || c == '>';
}

void Tokenizer::print_tokens() {
    for (const auto& tok : m_tokens) {
        std::visit(
            matches{[](const std::string& s) { std::println("Token: {}", s); },
                    [](int64_t num) { std::println("Number: {}", num); },
                    [](Symbol s) { std::println("{}", format_symbol(s)); }},
            tok);
    }
}
#endif

} // namespace tinydb
