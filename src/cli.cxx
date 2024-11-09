#include "cli.hxx"
#include <expected>
#include <iostream>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// [[maybe_unused]] to silence some of the warnings during development.

namespace tinydb {

Cli::Cli() = default;

/**
 * @brief Types of symbols recognized by the CLI.
 */
enum class Symbol : uint8_t {
    Equal,
    EqualEquals,
    LessThan,
    LessThanEqual,
    MoreThan,
    MoreThanEqual,
    Backslash,
};

/**
 * @brief Return debug prints from a symbol.
 *
 * @param s
 * @return
 */
constexpr auto format_symbol(Symbol s) -> std::string {
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

using Token = std::variant<std::string, Symbol>;

enum class State : uint8_t {
    Word,
    Quote,
    Equal,
    LessThan,
    MoreThan,
};

/**
 * @class Parser
 * @brief Parses an input string into tokens.
 *
 */
class Parser {
  public:
    enum class ParseError : uint8_t {
        UnknownCommand = 0,
        MissingArguments,
        SussySymbols,
        EmptyQuote,
    };

    Parser() = default;

    /**
     * @brief Take the list of tokens out of `Parser`.
     * After this function call, one should not use this `Parser` anymore, or
     * one should construct a new `Parser.`
     *
     * @return rvalue of tokens vector.
     */
    auto take_tokens() -> std::vector<Token>&& { return std::move(m_tokens); }

    /**
     * @brief Parses the input string.
     *
     * @param input 
     * @return Nothing if successful, ParseError if an error occurs.
     */
    auto parse(std::string_view input) -> std::expected<void, ParseError> {
        auto is_space = [](char c) {
            return c == ' ' || c == '\t' || c == '\n';
        };
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

    auto handle_state(char c) -> std::expected<void, ParseError> {
        switch (m_curr_state) {
        case State::Word:
            return handle_word(c);
        default:
            return {};
        }
        return {};
    }

    auto handle_word(char c) -> std::expected<void, ParseError> {
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

    auto handle_quote(char c) -> std::expected<void, ParseError> {
        if(c == '"') {
            if(m_curr_word.empty()) {
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

    // TODO: handle all the other equals

    /**
     * @brief Helper for variants
     *
     * @tparam Ts
     */
    template <class... Ts> struct matches : Ts... {
        using Ts::operator()...;
    };

#ifndef NDEBUG
    /**
     * @brief Debug printing
     */
    void print_tokens() {
        for (const auto& tok : m_tokens) {
            std::visit(
                matches{
                    [](const std::string& s) { std::println("Token: {}", s); },
                    [](Symbol s) { std::println("{}", format_symbol(s)); }},
                tok);
        }
    }
#endif // !NDEBUG
  private:
    std::vector<Token> m_tokens{};
    std::string m_curr_word{};
    State m_curr_state{State::Word};
};

void Cli::run() {
    std::print("> ");
    std::flush(std::cout);
    std::string input{};
    std::getline(std::cin, input);
    Parser ps{};
    ps.parse(input);
#ifndef NDEBUG
    ps.print_tokens();
#endif // !NDEBUG
}

} // namespace tinydb
