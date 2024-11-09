#ifndef TINYDB_PARSER_HXX
#define TINYDB_PARSER_HXX

#include <expected>
#include <variant>
#include <vector>

namespace tinydb {

enum class ParseError : uint8_t {
    UnknownCommand = 0,
    MissingArguments,
    SussySymbols,
    EmptyQuote,
};

// explicit instantiation to reduce compile time.
using ParserReturn = std::expected<void, ParseError>;

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

enum class State : uint8_t {
    Word,
    Quote,
    Equal,
    LessThan,
    MoreThan,
};

// explicit instantiation to reduce compile time.
using Token = std::variant<std::string, Symbol>;

/**
 * @class Parser
 * @brief Parses an input string into tokens.
 *
 */
class Parser {
  public:
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
    auto parse(std::string_view input) -> ParserReturn;

    /**
     * @brief State machine.
     *
     * @param c Next character.
     * @return Whether parsing succeeded.
     */
    auto handle_state(char c) -> ParserReturn {
        switch (m_curr_state) {
        case State::Word:
            return handle_word(c);
        case State::Quote:
            return handle_quote(c);
        default:
            return {};
        }
        return {};
    }

    auto handle_word(char c) -> ParserReturn; 

    auto handle_quote(char c) -> ParserReturn; 

    // TODO: handle all the other equals

#ifndef NDEBUG
    /**
     * @brief Debug printing
     */
    void print_tokens(); 
#endif // !NDEBUG
  private:
    std::string m_curr_word{};
    std::vector<Token> m_tokens{};
    State m_curr_state{State::Word};
};

} // namespace tinydb

#endif // !TINYDB_PARSER_HXX
