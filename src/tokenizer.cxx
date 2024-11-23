#include "tokenizer.hxx"
#include <cassert>
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

    if (!m_curr_word.empty()) {
        m_tokens.emplace_back(Identifier{std::move(m_curr_word)});
    }

    return {};
}

auto Tokenizer::handle_space(char c) -> StateHandleReturn {
    switch (c) {
    case ' ':
        return State::Space;
    default:
        return handle_word(c);
    }
}

auto Tokenizer::handle_word(char c) -> StateHandleReturn {
    // assert(!m_curr_word.empty());
    switch (c) {
    case ' ':
        m_tokens.emplace_back(Identifier{std::move(m_curr_word)});
        // reconstructs m_curr_word
        m_curr_word = {};
        return State::Space;
    case '=':
        m_tokens.emplace_back(Symbol::Equal);
        return State::Space;
    case '<':
        return State::LessThan;
    case '>':
        return State::MoreThan;
    case '\'':
        return State::Quote;
    case ',':
        m_tokens.emplace_back(Symbol::Comma);
        return State::Space;
    case ';':
        m_tokens.emplace_back(Symbol::Semicolon);
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
        std::visit(matches{[](const Identifier& iden) {
                               std::println("Identifier: {}", iden.val);
                           },
                           [&](const Literal& lit) { print_literal(lit); },
                           [](Symbol sym) {
                               std::print("Symbol: {}", format_symbol(sym));
                           }},
                   tok);
    }
}
#endif // !NDEBUG

} // namespace tinydb
