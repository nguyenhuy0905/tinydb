#include "parser.hxx"
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

auto format_symbol(Symbol s) -> std::string {
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

auto Parser::parse(std::string_view input) -> ParserReturn {
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
    }

    return {};
}

auto Parser::handle_word(char c) -> ParserReturn {
    if (c == ' ') {
        if (m_curr_word.empty()) {
            return {};
        }
        m_tokens.emplace_back(std::move(m_curr_word));
        // reconstruct m_curr_word
        m_curr_word = {};
        return {};
    }
    switch (c) {
    case '=':
        m_curr_state = State::Equal;
        return {};
    case '<':
        m_curr_state = State::LessThan;
        return {};
    case '>':
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

auto Parser::handle_quote(char c) -> ParserReturn {
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

void Parser::print_tokens() {
    for (const auto& tok : m_tokens) {
        std::visit(
            matches{[](const std::string& s) { std::println("Token: {}", s); },
                    [](Symbol s) { std::println("{}", format_symbol(s)); }},
            tok);
    }
}

} // namespace tinydb
