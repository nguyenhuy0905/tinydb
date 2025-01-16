#include "tokenizer.hxx"
#include <cassert>
#include <cctype>
#include <cstdint>
#include <print>
#include <ranges>
#include <string>
#include <string_view>

namespace tinydb {
/**
 * @brief Helper for variants
 *
 * @tparam Ts
 */
template <class... Ts> struct matches : Ts... {
    using Ts::operator()...;
};

[[maybe_unused]]
static auto format_symbol(Symbol s) -> std::string {
    std::string ret{"Symbol: "};
    switch (s) {
    case Symbol::Equal:
        ret.append("=");
        break;
    case Symbol::DashDash:
        ret.append("--");
        break;
    case Symbol::NotEqual:
        ret.append("<>");
        break;
    case Symbol::MoreThan:
        ret.append(">");
        break;
    case Symbol::LessThan:
        ret.append("<");
        break;
    case Symbol::LessThanEqual:
        ret.append("<=");
        break;
    case Symbol::MoreThanEqual:
        ret.append(">=");
        break;
    case Symbol::Comma:
        ret.append(",");
        break;
    case Symbol::Semicolon:
        ret.append(";");
        break;
    case Symbol::LParen:
        ret.append("(");
        break;
    case Symbol::RParen:
        ret.append(")");
        break;
    }

    return ret;
}

auto Tokenizer::tokenize(std::string_view input) -> TokenizerReturn {
    auto is_space = [](char c) { return c == ' ' || c == '\t' || c == '\n'; };
    auto trimmed = [&](std::string_view sv) {
        return sv | std::views::drop_while(is_space) | std::views::reverse |
               std::views::drop_while(is_space) | std::views::reverse;
    };

    for (const char c : trimmed(input)) {
        auto handle_result = handle_state(c).and_then([this](State s) {
            m_curr_state = s;
            return std::expected<void, TokenizerError>{};
        });
        if (!handle_result) {
            return std::unexpected(handle_result.error());
        }
    }
    switch (m_curr_state) {
    case State::Space:
    case State::Comment:
    case State::Word:
    case State::Number:
        return handle_space(' ').transform([](auto) { return; });
    case State::Quote:
        return std::unexpected{TokenizerError::MissingQuote};
    default:
        return std::unexpected{TokenizerError::SussySymbols};
    }
}

auto Tokenizer::handle_space(char c) -> StateHandleReturn {
    switch (c) {
    case ' ':
        if (!m_curr_word.empty()) {
            m_tokens.emplace_back(Identifier{std::move(m_curr_word)});
            m_curr_word = {};
        }
        if (m_curr_num) {
            m_tokens.emplace_back(Literal{m_curr_num.value()});
            m_curr_num.reset();
        }
        return State::Space;
    default:
        return handle_word(c);
    }
}

auto Tokenizer::handle_word(char c) -> StateHandleReturn {
    auto emplace_word = [this]() {
        if (!m_curr_word.empty()) {
            m_tokens.emplace_back(Identifier{std::move(m_curr_word)});
            m_curr_word = {};
        }
    };
    if (isdigit(c)) {
        assert(!m_curr_num);
        if (m_curr_word.empty()) {
            m_curr_num = (c - '0');
            return State::Number;
        }
    }
    switch (c) {
    case ' ':
        emplace_word();
        // reconstructs m_curr_word
        return State::Space;
    case '=':
        emplace_word();
        m_tokens.emplace_back(Symbol::Equal);
        return State::Space;
    case '<':
        emplace_word();
        return State::LeftPointy;
    case '>':
        emplace_word();
        return State::RightPointy;
    case '\'':
        emplace_word();
        return State::Quote;
    case ',':
        emplace_word();
        m_tokens.emplace_back(Symbol::Comma);
        return State::Space;
    case ';':
        emplace_word();
        m_tokens.emplace_back(Symbol::Semicolon);
        return State::Space;
    case '-':
        emplace_word();
        return State::Dash;
    case '(':
        emplace_word();
        m_tokens.emplace_back(Symbol::LParen);
        return State::Space;
    case ')':
        emplace_word();
        m_tokens.emplace_back(Symbol::RParen);
        return State::Space;
    default:
        m_curr_word.push_back(c);
        return State::Word;
    }
}

auto Tokenizer::handle_quote(char c) -> StateHandleReturn {
    if (c == '\'') {
        m_tokens.emplace_back(Literal{std::move(m_curr_word)});
        m_curr_word = {};
        return State::Space;
    }

    m_curr_word.push_back(c);
    return State::Quote;
}

auto Tokenizer::handle_left_pointy(char c) -> StateHandleReturn {
    switch (c) {
    case '<':
        return std::unexpected{TokenizerError::SussySymbols};
    case '>':
        m_tokens.emplace_back(Symbol::NotEqual);
        return State::Space;
    case '=':
        m_tokens.emplace_back(Symbol::LessThanEqual);
        return State::Space;
    default:
        m_tokens.emplace_back(Symbol::LessThan);
        return handle_space(c);
    }
}

auto Tokenizer::handle_right_pointy(char c) -> StateHandleReturn {
    switch (c) {
    case '>':
    case '<':
        return std::unexpected{TokenizerError::SussySymbols};
    case '=':
        m_tokens.emplace_back(Symbol::MoreThanEqual);
        return State::Space;
    default:
        m_tokens.emplace_back(Symbol::MoreThan);
        return handle_space(c);
    }
}

auto Tokenizer::handle_number(char c) -> StateHandleReturn {
    if (isdigit(c)) {
        m_curr_num = m_curr_num
                         .transform([c](int32_t i) {
                             // the goddamn 'magic number' warning.
                             constexpr int32_t ten = 10;
                             if (i < 0) {
                                 return (ten * i) - (c - '0');
                             }
                             return (ten * i) + (c - '0');
                         })
                         .or_else([this, c]() {
                             if (m_curr_state == State::Dash) {
                                 // the goddamn 'magic number' warning.
                                 return decltype(m_curr_num)(-(c - '0'));
                             }
                             return decltype(m_curr_num)(c - '0');
                         });
        return State::Number;
    }
    assert(m_curr_num);
    assert(m_curr_word.empty());
    if (isalpha(c) || c == '_') {
        // I know it's just a couple lines to hand-roll but I'm lazy.
        m_curr_word = std::to_string(m_curr_num.value());
        m_curr_num.reset();
        return handle_word(c);
    }
    m_tokens.emplace_back(Literal{m_curr_num.value()});
    m_curr_num.reset();
    return handle_space(c);
}

auto Tokenizer::handle_dash(char c) -> StateHandleReturn {
    if (c == '-') {
        return State::Comment;
    }
    if (c >= '0' && c <= '9') {
        return handle_number(c);
    }
    return std::unexpected{TokenizerError::SussySymbols};
}

#ifndef NDEBUG
void Tokenizer::print_tokens() {
    auto print_literal = [](const Literal& lit) {
        std::visit(matches{
                       [](const std::string& s) {
                           std::println("Literal string: '{}'", s);
                       },
                       [](int32_t i) { std::println("Literal number: {}", i); },
                       [](bool b) { std::println("Literal boolean: {}", b); },
                   },
                   lit.val);
    };
    for (const auto& tok : m_tokens) {
        std::visit(
            matches{[](const Identifier& iden) {
                        std::println("Identifier: {}", iden.val);
                    },
                    [&](const Literal& lit) { print_literal(lit); },
                    [](Symbol sym) { std::println("{}", format_symbol(sym)); }},
            tok);
    }
}
#endif // !NDEBUG

} // namespace tinydb
